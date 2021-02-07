#include "mcpch.h"
#include "IndexBuffer.h"

#include "RendererAPI.h"

#include "Mocha/Platform/Vulkan/VulkanIndexBuffer.h"

namespace Mocha {

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanIndexBuffer>::Create(size);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanIndexBuffer>::Create(data, size);
		}

		MC_ASSERT("Unknown RendererAPI");
		return nullptr;
	}

}