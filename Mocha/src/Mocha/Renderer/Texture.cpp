#include "mcpch.h"
#include "Texture.h"

#include "Mocha/Renderer/RendererAPI.h"
#include "Mocha/Platform/Vulkan/VulkanTexture.h"

namespace Mocha {

	Ref<Texture2D> Texture2D::Create(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanTexture2D>::Create(format, width, height, wrap);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanTexture2D>::Create(path, srgb);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB:	return 3;
			case TextureFormat::RGBA:	return 4;
		}

		return 0;
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;
	}

}