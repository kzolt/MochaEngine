#include "mcpch.h"
#include "Pipeline.h"

#include "Renderer.h"

#include "Mocha/Platform/Vulkan/VulkanPipeline.h"

namespace Mocha {

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:			return nullptr;
			case RendererAPIType::Vulkan:		return Ref<VulkanPipeline>::Create(spec);
		}

		MC_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}