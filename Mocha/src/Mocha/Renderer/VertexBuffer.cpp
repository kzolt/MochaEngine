#include "mcpch.h"
#include "VertexBuffer.h"

#include "RendererAPI.h"

#include "Mocha/Platform/Vulkan/VulkanVertexBuffer.h"

namespace Mocha {

	Ref<VertexBuffer> VertexBuffer::Create(void* data, uint32_t size, VertexBufferUsage usage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanVertexBuffer>::Create(data, size, usage);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return Ref<VulkanVertexBuffer>::Create(size, usage);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}