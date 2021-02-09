#pragma once

#include "Mocha/Renderer/Texture.h"
#include "Vulkan.h"

namespace Mocha {

	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path, bool srgb = false);
		VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap);
		virtual ~VulkanTexture2D();

		void Invalidate();
		
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_DescriptorImageInfo; }

		void Lock() override {}
		void Unlock() override {}

		const std::string& GetPath() const override { return m_Path; }
		Buffer GetWriteableBuffer() override { return m_ImageData; }

		void Resize(uint32_t width, uint32_t height = 0) override {}
		bool Loaded() const override { return true; }

		void Bind(uint32_t slot = 0) const override {}

		TextureFormat GetFormat() const override { return TextureFormat::None; }
		uint32_t GetMipLevelCount() const override { return 0; }

		bool operator ==(const Texture& other) const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;

		Buffer m_ImageData;

		VkDeviceMemory m_DeviceMemory;
		VkImage m_Image;

		VkDescriptorImageInfo m_DescriptorImageInfo{};
	};

}