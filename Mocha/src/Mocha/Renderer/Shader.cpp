#include "mcpch.h"
#include "Shader.h"

#include "Mocha/Platform/Vulkan/VulkanShader.h"

namespace Mocha {
	
	std::vector<Ref<Shader>> Shader::s_AllShaders;

	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		Ref<Shader> result = nullptr;

		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:		return nullptr;
		case RendererAPIType::Vulkan:	
			result = Ref<VulkanShader>::Create(filepath);
			break;
		}

		return result;
	}
}