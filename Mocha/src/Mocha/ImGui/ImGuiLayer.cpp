#include "mcpch.h"
#include "ImGuiLayer.h"

#include "Mocha/Renderer/RendererAPI.h"

#include "Mocha/Platform/Vulkan/VulkanImGuiLayer.h"

namespace Mocha {

	ImGuiLayer* ImGuiLayer::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:		return nullptr;
			case RendererAPIType::Vulkan:	return new VulkanImGuiLayer();
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}