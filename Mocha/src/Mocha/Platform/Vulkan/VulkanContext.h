#pragma once

#include "Mocha/Renderer/Renderer.h"
#include "Mocha/Renderer/RendererContext.h"

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include "VulkanSwapchain.h"

struct GLFWwindow;

namespace Mocha {

	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext();

		virtual void Create() override;
		virtual void SwapBuffers() override;

		virtual void OnResize(uint32_t width, uint32_t height) override;

		virtual void BeginFrame() override;

		Ref<VulkanDevice> GetDevice() { return m_Device; }
		VulkanSwapchain& GetSwapchain() { return m_Swapchain; }

	public:
		static VkInstance GetInstance() { return s_VulkanInstance; }

		static Ref<VulkanContext> Get() { return Ref<VulkanContext>(Renderer::GetContext()); }
		static Ref<VulkanDevice> GetCurrentDevice() { return Get()->GetDevice(); }

	private:
		GLFWwindow* m_WindowHandle;

		// Devices
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice> m_Device;

		// Vulkan Instance
		inline static VkInstance s_VulkanInstance;
		VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;
		VkPipelineCache m_PipelineCache;

		VulkanAllocator m_Allocator;
		VulkanSwapchain m_Swapchain;
	};

}