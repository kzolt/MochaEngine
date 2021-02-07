#include "mcpch.h"
#include "VulkanIndexBuffer.h"

#include "VulkanContext.h"

namespace Mocha {

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t size)
		: m_Size(size)
	{

	}

	VulkanIndexBuffer::VulkanIndexBuffer(void* data, uint32_t size)
		: m_Size(size)
	{
		m_LocalData = Buffer::Copy(data, size);

		auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		// Index Buffer
		VkBufferCreateInfo indexBufferInfo{};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.size = m_Size;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		// Copy index data to a buffer visible to the host
		VK_CHECK_RESULT(vkCreateBuffer(device, &indexBufferInfo, nullptr, &m_VulkanBuffer));
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, m_VulkanBuffer, &memoryRequirements);

		VulkanAllocator allocator("IndexBuffer");
		allocator.Allocate(memoryRequirements, &m_DeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* dstBuffer;
		VK_CHECK_RESULT(vkMapMemory(device, m_DeviceMemory, 0, m_Size, 0, &dstBuffer));
		memcpy(dstBuffer, m_LocalData.Data, m_Size);
		vkUnmapMemory(device, m_DeviceMemory);

		VK_CHECK_RESULT(vkBindBufferMemory(device, m_VulkanBuffer, m_DeviceMemory, 0));
	}

}