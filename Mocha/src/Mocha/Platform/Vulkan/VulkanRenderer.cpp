#include "mcpch.h"
#include "VulkanRenderer.h"

#include "Vulkan.h"
#include "VulkanContext.h"

#include "Mocha/Platform/Vulkan/VulkanPipeline.h"
#include "Mocha/Platform/Vulkan/VulkanVertexBuffer.h"
#include "Mocha/Platform/Vulkan/VulkanIndexBuffer.h"
#include "Mocha/Platform/Vulkan/VulkanFramebuffer.h"

#include "Mocha/Platform/Vulkan/VulkanShader.h"

#define IMGUI_IMPL_API
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"

namespace Mocha {

	static VkCommandBuffer s_ImGuiCommandBuffer;
	static VkCommandBuffer s_CompositeCommandBuffer;

	static Ref<Framebuffer> s_Framebuffer;
	static Ref<Pipeline> s_CompositePipeline;
	static Ref<VertexBuffer> s_QuadVertexBuffer;
	static Ref<IndexBuffer> s_QuadIndexBuffer;
	static VkDescriptorSet s_QuadDescriptorSet;

	void VulkanRenderer::Submit()
	{

	}

	void VulkanRenderer::OnResize()
	{
		auto framebuffer = s_CompositePipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer.As<VulkanFramebuffer>();

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = s_QuadDescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &framebuffer->GetVulkanDescriptorInfo();
		writeDescriptorSet.dstBinding = 0;

		auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		vkUpdateDescriptorSets(vulkanDevice, 1, &writeDescriptorSet, 0, nullptr);

	}

	void VulkanRenderer::Init()
	{
		s_ImGuiCommandBuffer = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();
		s_CompositeCommandBuffer = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();

		/*
		{
			FramebufferSpecification spec;
			s_Framebuffer = Framebuffer::Create(spec);

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" }
			};
			
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = s_Framebuffer;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			s_CompositePipeline = Pipeline::Create(pipelineSpecification);
		}
		*/
	}

	static void CompositeRenderPass(VkCommandBufferInheritanceInfo& inheritanceInfo)
	{

	}

	void VulkanRenderer::Draw()
	{
		Ref<VulkanContext> context = VulkanContext::Get();
		VulkanSwapchain& swapchain = context->GetSwapchain();

		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = swapchain.GetCurrentDrawCommandBuffer();
		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffer, &cmdBufInfo));

		Ref<VulkanFramebuffer> framebuffer = s_Framebuffer.As<VulkanFramebuffer>();

		uint32_t width = framebuffer->GetSpecification().Width;
		uint32_t height = framebuffer->GetSpecification().Height;

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = framebuffer->GetVulkanFramebuffer();

		vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

		// Update dynamic scissor data
		VkRect2D scissor{};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

		// TODO: Render Stuff

		vkCmdEndRenderPass(drawCommandBuffer);
	}

	void VulkanRenderer::DrawImGui()
	{
		Ref<VulkanContext> context = VulkanContext::Get();
		VulkanSwapchain& swapchain = context->GetSwapchain();

		VkCommandBuffer drawCommandBuffer = swapchain.GetCurrentDrawCommandBuffer();

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapchain.GetWidth();
		uint32_t height = swapchain.GetHeight();

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapchain.GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = swapchain.GetCurrentFramebuffer();

		{
			vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			VkCommandBufferInheritanceInfo inheritanceInfo{};
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.renderPass = swapchain.GetRenderPass();
			inheritanceInfo.framebuffer = swapchain.GetCurrentFramebuffer();

			std::vector<VkCommandBuffer> commandBuffers;
			//CompositeRenderPass(inheritanceInfo);
			//commandBuffers.push_back(s_CompositeCommandBuffer);

			// ImGui Pass
			{
				VkCommandBufferBeginInfo cmdBufInfo{};
				cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

				VK_CHECK_RESULT(vkBeginCommandBuffer(s_ImGuiCommandBuffer, &cmdBufInfo));

				// Update dynamic viewport state
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)width;
				viewport.height = (float)height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(s_ImGuiCommandBuffer, 0, 1, &viewport);

				// Update dynamic scissor data
				VkRect2D scissor{};
				scissor.extent.width = width;
				scissor.extent.height = height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				vkCmdSetScissor(s_ImGuiCommandBuffer, 0, 1, &scissor);

				ImGui::Render();
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_ImGuiCommandBuffer);

				VK_CHECK_RESULT(vkEndCommandBuffer(s_ImGuiCommandBuffer));

				commandBuffers.push_back(s_ImGuiCommandBuffer);
			}

			vkCmdExecuteCommands(drawCommandBuffer, commandBuffers.size(), commandBuffers.data());
			vkCmdEndRenderPass(drawCommandBuffer);
		}

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffer));
	}

}