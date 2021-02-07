#pragma once

#include "Mocha/Renderer/VertexBuffer.h"
#include "Mocha/Core/Buffer.h"

#include "VulkanAllocator.h"

namespace Mocha {

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
		VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);

		virtual ~VulkanVertexBuffer() {}

		virtual void SetData(void* buffer, uint32_t, uint32_t offset = 0) override {}
		virtual void Bind() const override {}

		virtual const VertexBufferLayout& GetLayout() const override { return {}; }
		virtual void SetLayout(const VertexBufferLayout& layout) override {}

		virtual uint32_t GetSize() const override { return m_Size; }

		VkBuffer GetVulkanBuffer() { return m_VulkanBuffer; }
	private:
		uint32_t m_Size = 0;
		Buffer m_LocalData;
		
		VkBuffer m_VulkanBuffer;
		VkDeviceMemory m_DeviceMemory;
	};

}