#include "mcpch.h"
#include "RendererContext.h"

#include "Mocha/Platform/Vulkan/VulkanContext.h"

#include "Mocha/Renderer/RendererAPI.h"

namespace Mocha {

	Ref<RendererContext> RendererContext::Create(GLFWwindow* windowHandle)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:		return nullptr;
		case RendererAPIType::Vulkan:	return Ref<VulkanContext>::Create(windowHandle);
		}

		MC_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}