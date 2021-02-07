#include "mcpch.h"
#include "RenderPass.h"

#include "Mocha/Platform/Vulkan/VulkanRenderPass.h"

namespace Mocha {

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:		MC_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::Vulkan:	return Ref<VulkanRenderPass>::Create(spec);
		}
	}

}