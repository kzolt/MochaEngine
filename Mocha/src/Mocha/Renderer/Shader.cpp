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

	ShaderUniform::ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset)
		: m_Name(name), m_Type(type), m_Size(size), m_Offset(offset)
	{
	}

	const std::string& ShaderUniform::UniformTypeToString(ShaderUniformType type)
	{
		switch (type)
		{
			case ShaderUniformType::Bool:		return "Boolean";
			case ShaderUniformType::Int:		return "Int";
			case ShaderUniformType::Float:		return "Float";
		}

		return "None";
	}

}