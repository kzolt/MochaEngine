#pragma once

#include "Mocha/Renderer/Framebuffer.h"

#include "Vulkan.h"

namespace Mocha {

	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer();

		virtual void Bind() const override {}
		virtual void BindTexture(uint32_t slot = 0) const override {}

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;

		const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_DescriptorImageInfo; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkFramebuffer GetVulkanFramebuffer() const { return m_Framebuffer; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;

		struct FramebufferAttachment
		{
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		};

		FramebufferAttachment m_ColorAttachment, m_DepthAttachment;
		VkSampler m_ColorAttachmentSampler;
		VkRenderPass m_RenderPass;
		VkFramebuffer m_Framebuffer = nullptr;
		VkDescriptorImageInfo m_DescriptorImageInfo;
	};

}