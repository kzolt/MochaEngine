#include "mcpch.h"
#include "VulkanSwapchain.h"

#include <GLFW/glfw3.h>

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)															\
{																											\
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk"#entrypoint));	\
	MC_ASSERT(fp##entrypoint);																				\
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)															\
{																										\
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk"#entrypoint));   \
	MC_ASSERT(fp##entrypoint);																			\
}

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
static PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
static PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
static PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
static PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
static PFN_vkQueuePresentKHR fpQueuePresentKHR;

namespace Mocha {

	void VulkanSwapchain::Init(VkInstance instance, const Ref<VulkanDevice>& device)
	{
		m_Instance = instance;
		m_Device = device;
		m_Allocator = VulkanAllocator(device, "Swapchain");

		VkDevice vulkanDevice = m_Device->GetVulkanDevice();
		GET_DEVICE_PROC_ADDR(vulkanDevice, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, QueuePresentKHR);

		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	}

	void VulkanSwapchain::InitSurface(GLFWwindow* windowHandle)
	{
		VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface);

		// Get available queue family properties
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
		MC_ASSERT(queueCount >= 1);

		std::vector<VkQueueFamilyProperties> queueProps(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		std::vector<VkBool32> supportsPresent(queueCount);
		for (uint32_t i = 0; i < queueCount; i++)
			fpGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]);

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
					graphicsQueueNodeIndex = i;
				
				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		if (presentQueueNodeIndex == UINT32_MAX)
		{
			// If there's no queue that supports both present and graphics
			// Try to find a separate present queue
			for (uint32_t i = 0; i < queueCount; ++i)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		MC_ASSERT(graphicsQueueNodeIndex != UINT32_MAX);
		MC_ASSERT(presentQueueNodeIndex != UINT32_MAX);

		m_QueueNodeIndex = graphicsQueueNodeIndex;

		FindImageFormatAndColorSpace();
	}

	void VulkanSwapchain::Create(uint32_t* width, uint32_t* height, bool vsync)
	{
		VkDevice device = m_Device->GetVulkanDevice();
		VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		VkSwapchainKHR oldSwapchain = m_Swapchain;

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surfCaps;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps));

		// Get available present modes
		uint32_t presentModeCount;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, NULL));
		MC_ASSERT(presentModeCount > 0);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()));

		VkExtent2D swapchainExtent{};
		// If width (and height) equals the special value 0xffffffff, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested
			swapchainExtent.width = *width;
			swapchainExtent.height = *height;
		}
		else
		{
			// If the surface size is defined, the swapchain size must match
			swapchainExtent = surfCaps.currentExtent;
			*width = surfCaps.currentExtent.width;
			*height = surfCaps.currentExtent.height;
		}

		m_Width = *width;
		m_Height = *height;

		// Select a present mode for the swapchain

		// the VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency not-tearing present mode available
		if (!vsync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = surfCaps.currentTransform;

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.pNext = NULL;
		swapchainInfo.surface = m_Surface;
		swapchainInfo.minImageCount = desiredNumberOfSwapchainImages;
		swapchainInfo.imageFormat = m_ColorFormat;
		swapchainInfo.imageColorSpace = m_ColorSpace;
		swapchainInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = NULL;
		swapchainInfo.presentMode = swapchainPresentMode;
		swapchainInfo.oldSwapchain = oldSwapchain;
		// Setting clipped to VK_TRUE allows the implantation to discard rendering outside of the surface area
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Enable transfer destination on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VK_CHECK_RESULT(fpCreateSwapchainKHR(device, &swapchainInfo, nullptr, &m_Swapchain));

		// If an existing swap chain is re-created, destroy the old swap chain
		// This also cleans up all the presentable images
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_ImageCount; i++)
				vkDestroyImageView(device, m_Buffers[i].View, nullptr);
			fpDestroySwapchainKHR(device, oldSwapchain, nullptr);
		}
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, NULL));

		// Get the swapchain images
		m_Images.resize(m_ImageCount);
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, m_Images.data()));

		// Get the swap chain buffers containing the image and image view
		m_Buffers.resize(m_ImageCount);
		for (uint32_t i = 0; i < m_ImageCount; i++)
		{
			VkImageViewCreateInfo colorAttachmentView{};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = m_ColorFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			m_Buffers[i].Image = m_Images[i];
			colorAttachmentView.image = m_Buffers[i].Image;

			VK_CHECK_RESULT(vkCreateImageView(device, &colorAttachmentView, nullptr, &m_Buffers[i].View));
		}

		CreateDrawBuffers();

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Synchronization Objects
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphores.PresentComplete));
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphores.RenderComplete));

		// Set up submit info structure
		// Semaphores will stay the same during application lifetime
		// Command buffer submission info is set by each example
		VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		m_SubmitInfo = {};
		m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		m_SubmitInfo.pWaitDstStageMask = &pipelineStageFlags;
		m_SubmitInfo.waitSemaphoreCount = 1;
		m_SubmitInfo.pWaitSemaphores = &m_Semaphores.PresentComplete;
		m_SubmitInfo.signalSemaphoreCount = 1;
		m_SubmitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;

		// Wait fences to sync command buffer access
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_WaitFences.resize(m_DrawCommandBuffers.size());
		for (auto& fence : m_WaitFences)
			VK_CHECK_RESULT(vkCreateFence(m_Device->GetVulkanDevice(), &fenceCreateInfo, nullptr, &fence));

		CreateDepthStencil();

		VkFormat depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();

		// Render Pass
		std::array<VkAttachmentDescription, 2> attachments{};
		// Color attachment
		attachments[0].format = m_ColorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1].format = depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference{};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference{};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		//subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;// static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(m_Device->GetVulkanDevice(), &renderPassInfo, nullptr, &m_RenderPass));

		CreateFramebuffer();
	}

	void VulkanSwapchain::OnResize(uint32_t width, uint32_t height)
	{
		MC_CORE_WARN("VulkanContext::OnResize");
		auto device = m_Device->GetVulkanDevice();

		vkDeviceWaitIdle(device);

		Create(&width, &height);
		// Recreate the frame buffers
		vkDestroyImageView(device, m_DepthStencil.ImageView, nullptr);
		vkDestroyImage(device, m_DepthStencil.Image, nullptr);
		vkFreeMemory(device, m_DepthStencil.DeviceMemory, nullptr);
		CreateDepthStencil();

		for (auto& framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		CreateFramebuffer();

		// Command buffers need to be recreated as they may store
		// references to the recreated frame buffer
		vkFreeCommandBuffers(device, m_CommandPool, static_cast<uint32_t>(m_DrawCommandBuffers.size()), m_DrawCommandBuffers.data());
		CreateDrawBuffers();

		vkDeviceWaitIdle(device);
	}

	void VulkanSwapchain::BeginFrame()
	{
		VK_CHECK_RESULT(AcquireNextImage(m_Semaphores.PresentComplete, &m_CurrentBufferIndex));
	}

	void VulkanSwapchain::Present()
	{
		const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

		// Use a fence to wait until the command buffer has finished execution before using it again
		VK_CHECK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]));

		// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// The submit info structure specifies a command buffer queue submission batch
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.pWaitSemaphores = &m_Semaphores.PresentComplete;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &m_DrawCommandBuffers[m_CurrentBufferIndex];
		submitInfo.commandBufferCount = 1;

		// Submit to the graphics queue passing a wait fence
		VK_CHECK_RESULT(vkQueueSubmit(m_Device->GetQueue(), 1, &submitInfo, m_WaitFences[m_CurrentBufferIndex]));

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as thee wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted

		VkResult result = QueuePresent(m_Device->GetQueue(), m_CurrentBufferIndex, m_Semaphores.RenderComplete);

		if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				// swap chain is no longer compatible with the surface and needs to be recreated
				OnResize(m_Width, m_Height);
				return;
			}
			else
			{
				VK_CHECK_RESULT(result);
			}
		}

		VK_CHECK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, DEFAULT_FENCE_TIMEOUT));
	}


	void VulkanSwapchain::Cleanup()
	{
		VkDevice device = m_Device->GetVulkanDevice();

		if (m_Swapchain)
		{
			for (uint32_t i = 0; i < m_ImageCount; i++)
				vkDestroyImageView(device, m_Buffers[i].View, nullptr);
		}

		if (m_Surface)
		{
			fpDestroySwapchainKHR(device, m_Swapchain, nullptr);
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		}

		m_Surface = VK_NULL_HANDLE;
		m_Swapchain = VK_NULL_HANDLE;
	}

	VkResult VulkanSwapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return fpAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_Swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
	}

	VkResult VulkanSwapchain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore/*= VK_NULL_HANDLE*/)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pImageIndices = &imageIndex;

		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}

		return fpQueuePresentKHR(queue, &presentInfo);
	}

	void VulkanSwapchain::CreateFramebuffer()
	{
		// Setup framebuffer
		VkImageView ivAttachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		ivAttachments[1] = m_DepthStencil.ImageView;

		VkFramebufferCreateInfo frameBufferCrateInfo{};
		frameBufferCrateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCrateInfo.pNext = NULL;
		frameBufferCrateInfo.renderPass = m_RenderPass;
		frameBufferCrateInfo.attachmentCount = 1;
		frameBufferCrateInfo.pAttachments = ivAttachments;
		frameBufferCrateInfo.width = m_Width;
		frameBufferCrateInfo.height = m_Height;
		frameBufferCrateInfo.layers = 1;

		// Create framebuffers for every swapchain images
		m_Framebuffers.resize(m_ImageCount);
		for (uint32_t i = 0; i < m_Framebuffers.size(); i++)
		{
			ivAttachments[0] = m_Buffers[i].View;
			VK_CHECK_RESULT(vkCreateFramebuffer(m_Device->GetVulkanDevice(), &frameBufferCrateInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanSwapchain::CreateDepthStencil()
	{
		VkFormat depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = depthFormat;
		imageCreateInfo.extent = { m_Width, m_Height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkDevice device = m_Device->GetVulkanDevice();
		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &m_DepthStencil.Image));
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(device, m_DepthStencil.Image, &memReqs);
		m_Allocator.Allocate(memReqs, &m_DepthStencil.DeviceMemory);
		VK_CHECK_RESULT(vkBindImageMemory(device, m_DepthStencil.Image, m_DepthStencil.DeviceMemory, 0));

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.image = m_DepthStencil.Image;
		imageViewCreateInfo.format = depthFormat;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats
		if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
			imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_DepthStencil.ImageView));
	}

	void VulkanSwapchain::CreateDrawBuffers()
	{
		// Create one command buffer for each swapchain image and reuse for rendering
		m_DrawCommandBuffers.resize(m_ImageCount);

		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = m_QueueNodeIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(m_Device->GetVulkanDevice(), &cmdPoolInfo, nullptr, &m_CommandPool));

		VkCommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool = m_CommandPool;
		commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_DrawCommandBuffers.size());
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device->GetVulkanDevice(), &commandBufferAllocInfo, m_DrawCommandBuffers.data()));
	}

	void VulkanSwapchain::FindImageFormatAndColorSpace()
	{
		VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		// Get list of supported surface formats
		uint32_t formatCount;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, NULL));
		MC_ASSERT(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data()));

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			m_ColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_ColorFormat = surfaceFormat.format;
					m_ColorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_ColorFormat = surfaceFormats[0].format;
				m_ColorSpace = surfaceFormats[0].colorSpace;
			}
		}


	}

}