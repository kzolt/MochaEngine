#include "mcpch.h"
#include "VulkanImGuiLayer.h"

#include "imgui.h"

#define IMGUI_IMPL_API
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"

#include "Mocha/Core/Application.h"

#include "Mocha/Platform/Vulkan/VulkanContext.h"

namespace Mocha {

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;

		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	VulkanImGuiLayer::VulkanImGuiLayer()
	{

	}

	VulkanImGuiLayer::VulkanImGuiLayer(const std::string& name)
	{

	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{

	}

	void VulkanImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui Context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImFont* pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
		io.FontDefault = io.Fonts->Fonts.back();

		// Setup Dear ImGUi style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		auto vulkanContext = VulkanContext::Get();
		auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkDescriptorPool descriptorPool;

		// Create Descriptor Pool
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		
		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		auto err = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
		check_vk_result(err);

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance = VulkanContext::GetInstance();
		init_info.PhysicalDevice = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
		init_info.Queue = VulkanContext::GetCurrentDevice()->GetQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = vulkanContext->GetSwapchain().GetImageCount();
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info, vulkanContext->GetSwapchain().GetRenderPass());

		{
			VkCommandBuffer commandBuffer = vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			vulkanContext->GetCurrentDevice()->FlushCommandBuffer(commandBuffer);

			err = vkDeviceWaitIdle(device);
			check_vk_result(err);
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	void VulkanImGuiLayer::OnDetach()
	{
		auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		auto err = vkDeviceWaitIdle(device);
		check_vk_result(err);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
	}

	void VulkanImGuiLayer::End()
	{
		ImGui::EndFrame();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{

	}

}