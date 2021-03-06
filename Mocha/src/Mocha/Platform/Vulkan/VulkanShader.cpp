#include "mcpch.h"
#include "VulkanShader.h"

#include "VulkanContext.h"
#include "VulkanAllocator.h"

#include <filesystem>
#include <fstream>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Mocha {

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	static std::string ReadShaderFromFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
		}
		else
		{
			MC_CORE_ASSERT(false, "Could not load shader!");
		}

		in.close();
		return result;
	}

	static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")							return VK_SHADER_STAGE_VERTEX_BIT;
		if (type == "fragment" || type == "pixel")		return VK_SHADER_STAGE_FRAGMENT_BIT;


		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	static ShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
			case spirv_cross::SPIRType::Boolean:	return ShaderUniformType::Bool;
			case spirv_cross::SPIRType::Int:		return ShaderUniformType::Int;
			case spirv_cross::SPIRType::Float:
				if (type.vecsize == 1)				return ShaderUniformType::Float;
				if (type.vecsize == 2)				return ShaderUniformType::Vec2;
				if (type.vecsize == 3)				return ShaderUniformType::Vec3;
				if (type.vecsize == 4)				return ShaderUniformType::Vec4;

				if (type.columns == 3)				return ShaderUniformType::Mat3;
				if (type.columns == 4)				return ShaderUniformType::Mat4;
				break;
		}
		MC_CORE_ASSERT("Unknown Type!");
		return ShaderUniformType::None;
	}

	//////////////////////////////////////////////////////////////////////////
	// Vulkan Shaders
	//////////////////////////////////////////////////////////////////////////

	VulkanShader::VulkanShader(const std::string& path)
		: m_AssetPath(path)
	{
		size_t found = path.find_last_of("/\\");
		m_Name = found != std::string::npos ? path.substr(found + 1) : path;
		found = m_Name.find_last_of(".");
		m_Name = found != std::string::npos ? m_Name.substr(0, found) : m_Name;

		Reload();
	}

	VulkanShader::~VulkanShader()
	{

	}

	void VulkanShader::Reload()
	{
		std::string source = ReadShaderFromFile(m_AssetPath);

		m_ShaderSource = PreProcess(source);
		m_ShaderStages.resize(2);

		std::array<std::vector<uint32_t>, 2> shaderData;
		CommpileOrGetVulkanBinary(shaderData, false);
		LoadAndCreateVertexShader(m_ShaderStages[0], shaderData[0]);
		LoadAndCreateFragmentShader(m_ShaderStages[1], shaderData[1]);

		Reflect(VK_SHADER_STAGE_VERTEX_BIT, shaderData[0]);
		Reflect(VK_SHADER_STAGE_FRAGMENT_BIT, shaderData[1]);

		CreateDescriptors();

	}

	void VulkanShader::Bind()
	{

	}

	RendererID VulkanShader::GetRendererID() const
	{
		return 0;
	}

	void* VulkanShader::MapUniformBuffer(uint32_t bindingPoint)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		uint8_t* pData;
		VK_CHECK_RESULT(vkMapMemory(device, m_UniformBuffers[bindingPoint].Memory, 0, m_UniformBuffers[bindingPoint].Size, 0, (void**)&pData));
		return pData;
	}

	void VulkanShader::UnmapUniformBuffer(uint32_t bindingPoint)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		vkUnmapMemory(device, m_UniformBuffers[bindingPoint].Memory);
	}

	//////////////////////////////////////////////////////////////////////////
	// Vulkan Uniforms
	//////////////////////////////////////////////////////////////////////////

	void VulkanShader::SetUniformBuffer(const std::string& name, const void* data, uint32_t size)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, float value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, int value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, const glm::vec2& value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, const glm::vec3& value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, const glm::vec4& value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, const glm::mat3& value)
	{

	}

	void VulkanShader::SetUniform(const std::string& name, const glm::mat4& value)
	{

	}

	void VulkanShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	// Shader Processing
	//////////////////////////////////////////////////////////////////////////

	std::unordered_map<VkShaderStageFlagBits, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;


		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			MC_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			MC_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			auto shaderType = ShaderTypeFromString(type);
			shaderSources[shaderType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void VulkanShader::CommpileOrGetVulkanBinary(std::array<std::vector<uint32_t>, 2>& outputBinary, bool forceCompile)
	{
		// Vertex Shader
		{
			std::filesystem::path p = m_AssetPath;
			auto path = p.parent_path() / "cached" / (p.filename().string() + ".cached_vulkan.vert");
			std::string cachedFilePath = path.string();

			FILE* f = fopen(cachedFilePath.c_str(), "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				fseek(f, 0, SEEK_SET);
				outputBinary[0] = std::vector<uint32_t>(size / sizeof(uint32_t));
				fread(outputBinary[0].data(), sizeof(uint32_t), outputBinary[0].size(), f);
				fclose(f);
			}
		}

		// Fragment Shader
		{
			std::filesystem::path p = m_AssetPath;
			auto path = p.parent_path() / "cached" / (p.filename().string() + ".cached_vulkan.frag");
			std::string cachedFilePath = path.string();

			FILE* f = fopen(cachedFilePath.c_str(), "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				fseek(f, 0, SEEK_SET);
				outputBinary[1] = std::vector<uint32_t>(size / sizeof(uint32_t));
				fread(outputBinary[1].data(), sizeof(uint32_t), outputBinary[1].size(), f);
				fclose(f);
			}
		}

		if (outputBinary[0].size() == 0)
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

			const bool optimize = false;
			if (optimize)
				options.SetOptimizationLevel(shaderc_optimization_level_performance);

			// Vertex Shader
			{
				auto& shaderSource = m_ShaderSource.at(VK_SHADER_STAGE_VERTEX_BIT);
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderSource, shaderc_vertex_shader, m_AssetPath.c_str(), options);

				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					MC_CORE_ERROR(module.GetErrorMessage());
					MC_CORE_ASSERT(false);
				}

				const uint8_t* begin = (const uint8_t*)module.cbegin();
				const uint8_t* end = (const uint8_t*)module.cend();
				const ptrdiff_t size = end - begin;

				outputBinary[0] = std::vector<uint32_t>(module.cbegin(), module.cend());
			}

			{
				std::filesystem::path p = m_AssetPath;
				auto path = p.parent_path() / "cached" / (p.filename().string() + ".cached_vulkan.vert");
				std::string cachedFilePath = path.string();

				FILE* f = fopen(cachedFilePath.c_str(), "wb");
				fwrite(outputBinary[0].data(), sizeof(uint32_t), outputBinary[0].size(), f);
				fclose(f);
			}
		}

		if (outputBinary[1].size() == 0)
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

			const bool optimize = false;
			if (optimize)
				options.SetOptimizationLevel(shaderc_optimization_level_performance);

			// Fragment Shader
			{
				auto& shaderSource = m_ShaderSource.at(VK_SHADER_STAGE_FRAGMENT_BIT);
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderSource, shaderc_fragment_shader, m_AssetPath.c_str(), options);

				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					MC_CORE_ERROR(module.GetErrorMessage());
					MC_CORE_ASSERT(false);
				}

				const uint8_t* begin = (const uint8_t*)module.cbegin();
				const uint8_t* end = (const uint8_t*)module.cend();
				const ptrdiff_t size = end - begin;

				outputBinary[1] = std::vector<uint32_t>(module.cbegin(), module.cend());
			}

			{
				std::filesystem::path p = m_AssetPath;
				auto path = p.parent_path() / "cached" / (p.filename().string() + ".cached_vulkan.frag");
				std::string cachedFilePath = path.string();

				FILE* f = fopen(cachedFilePath.c_str(), "wb");
				fwrite(outputBinary[1].data(), sizeof(uint32_t), outputBinary[1].size(), f);
				fclose(f);
			}
		}
	}

	void VulkanShader::LoadAndCreateVertexShader(VkPipelineShaderStageCreateInfo& shaderStage, const std::vector<uint32_t>& shaderData)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		MC_CORE_ASSERT(shaderData.size());

		// Create a new shader module that will be used for pipeline creation
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = shaderData.size() * sizeof(uint32_t);
		moduleCreateInfo.pCode = shaderData.data();

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStage.module = shaderModule;
		shaderStage.pName = "main";

	}

	void VulkanShader::LoadAndCreateFragmentShader(VkPipelineShaderStageCreateInfo& shaderStage, const std::vector<uint32_t>& shaderData)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		MC_CORE_ASSERT(shaderData.size());

		// Create a new shader module that will be used for pipeline creation
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = shaderData.size() * sizeof(uint32_t);
		moduleCreateInfo.pCode = shaderData.data();

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStage.module = shaderModule;
		shaderStage.pName = "main";
	}

	void VulkanShader::Reflect(VkShaderStageFlagBits shaderStage, const std::vector<uint32_t>& shaderData)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		MC_CORE_TRACE("==========================");
		MC_CORE_TRACE(" Vulkan Shader Reflection");
		MC_CORE_TRACE(" {0}", m_AssetPath);
		MC_CORE_TRACE("==========================");

		// Vertex Shader
		spirv_cross::Compiler compiler(shaderData);
		auto resources = compiler.get_shader_resources();

		MC_CORE_TRACE("Uniform Buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			int memberCount = bufferType.member_types.size();
			uint32_t bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t size = compiler.get_declared_struct_size(bufferType);

			MC_CORE_ASSERT(m_UniformBuffers.find(bindingPoint) == m_UniformBuffers.end());
			UniformBuffer& buffer = m_UniformBuffers[bindingPoint];
			buffer.BindingPoint = bindingPoint;
			buffer.Size = size;
			buffer.Name = name;
			buffer.ShaderStage = shaderStage;

			MC_CORE_TRACE("  Name: {0}", name);
			MC_CORE_TRACE("  Member Count: {0}", memberCount);
			MC_CORE_TRACE("  Binding Point: {0}", bindingPoint);
			MC_CORE_TRACE("  Size: {0}", size);
			MC_CORE_TRACE("-------------------------");
		}

		MC_CORE_TRACE("Push Constant Buffers:");
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& bufferName = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = compiler.get_declared_struct_size(bufferType);
			int memberCount = bufferType.member_types.size();
			uint32_t size = compiler.get_declared_struct_size(bufferType);
			uint32_t offset = 0;
		
			if (m_PushConstantRanges.size())
				offset = m_PushConstantRanges.back().Offset + m_PushConstantRanges.back().Size;
		
			auto& pushConstantRange = m_PushConstantRanges.emplace_back();
			pushConstantRange.ShaderStage = shaderStage;
			pushConstantRange.Size = bufferSize;
			pushConstantRange.Offset = offset;
		
			// Skip empty push constant buffers
			if (bufferName.empty())
				continue;
		
			ShaderBuffer& buffer = m_Buffers[bufferName];
			buffer.Name = bufferName;
			buffer.Size = bufferSize;
		
			MC_CORE_TRACE("  Name: {0}", bufferName);
			MC_CORE_TRACE("  Member Count: {0}", memberCount);
			MC_CORE_TRACE("  Size: {0}", size);
		
			for (int i = 0; i < memberCount; i++)
			{
				auto type = compiler.get_type(bufferType.member_types[i]);
				const auto& memberName = compiler.get_member_name(bufferType.self, i);
				auto size = compiler.get_declared_struct_member_size(bufferType, i);
				auto offset = compiler.type_struct_member_offset(bufferType, i);
		
				std::string uniformName = bufferName + "." + memberName;
				buffer.Uniforms[uniformName] = ShaderUniform(uniformName, SPIRTypeToShaderUniformType(type), size, offset);
			}
		}
		
		MC_CORE_TRACE("Sampled Images: ");
		for (const auto& resource : resources.sampled_images)
		{
			const auto& name = resource.name;
			auto& type = compiler.get_type(resource.base_type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t dimention = type.image.dim;
		
			auto& imageSampler = m_ImageSamplers[binding];
			imageSampler.BindingPoint = binding;
			imageSampler.Name = name;
			imageSampler.ShaderStage = shaderStage;
		}

		MC_CORE_TRACE("==========================");

	}

	void VulkanShader::CreateDescriptors()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		//////////////////////////////////////////////////////////////////////
		// Descriptor Pool
		//////////////////////////////////////////////////////////////////////

		// We need to tell the API the number of max requested descriptors per type
		std::vector<VkDescriptorPoolSize> typeCounts;
		if (m_UniformBuffers.size())
		{
			VkDescriptorPoolSize& typeCount = typeCounts.emplace_back();
			typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			typeCount.descriptorCount = m_UniformBuffers.size();
		}

		if (m_ImageSamplers.size())
		{
			VkDescriptorPoolSize& typeCount = typeCounts.emplace_back();
			typeCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			typeCount.descriptorCount = m_ImageSamplers.size();
		}

		// TODO: Move this to the centralized renderer
		// Create the global desriptor pool
		// All descriptors used in this example are allocated from this pool
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = typeCounts.size();
		descriptorPoolInfo.pPoolSizes = typeCounts.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_DescriptorPool));

		//////////////////////////////////////////////////////////////////////
		// Descriptor Set Layout
		//////////////////////////////////////////////////////////////////////
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		for (auto& [binding, uniformBuffer] : m_UniformBuffers)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = uniformBuffer.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			VkWriteDescriptorSet& set = m_WriteDescriptorSets[uniformBuffer.Name];
			set = {};
			set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			set.descriptorType = layoutBinding.descriptorType;
			set.descriptorCount = 1;
			set.dstBinding = layoutBinding.binding;

			AllocateUniformBuffer(uniformBuffer);
		}

		for (auto& [binding, imageSampler] : m_ImageSamplers)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.binding = binding;

			MC_CORE_ASSERT(m_UniformBuffers.find(binding) == m_UniformBuffers.end(), "Binding is already present!");

			VkWriteDescriptorSet& set = m_WriteDescriptorSets[imageSampler.Name];
			set = {};
			set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			set.descriptorType = layoutBinding.descriptorType;
			set.descriptorCount = 1;
			set.dstBinding = layoutBinding.binding;
		}

		VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
		descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayout.pNext = nullptr;
		descriptorLayout.bindingCount = layoutBindings.size();
		descriptorLayout.pBindings = layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanShader::AllocateUniformBuffer(UniformBuffer& dst)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		
		UniformBuffer& uniformBuffer = dst;

		// Vertex shader uniform buffer block
		VkBufferCreateInfo bufferInfo{};
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = uniformBuffer.Size;
		// This buffer will be used as a uniform buffer
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;


		VulkanAllocator allocator("UniformBuffer");

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &uniformBuffer.Buffer));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, uniformBuffer.Buffer, &memoryRequirements);
		allocInfo.allocationSize = memoryRequirements.size;

		allocator.Allocate(memoryRequirements, &uniformBuffer.Memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkBindBufferMemory(device, uniformBuffer.Buffer, uniformBuffer.Memory, 0));

		// Store information in the uniform's descriptor that is used by the descriptor set
		uniformBuffer.Descriptor.buffer = uniformBuffer.Buffer;
		uniformBuffer.Descriptor.offset = 0;
		uniformBuffer.Descriptor.range = uniformBuffer.Size;
	}

	VkDescriptorSet VulkanShader::CreateDescriptorSet()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkDescriptorSet descriptorSet;

		// Allocate a new descriptor set from the global descriptor pool
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
		return descriptorSet;
	}

	const VkWriteDescriptorSet* VulkanShader::GetDescriptorSet(const std::string& name) const
	{
		if (m_WriteDescriptorSets.find(name) == m_WriteDescriptorSets.end())
		{
			MC_CORE_WARN("Shader {0} does not contain requested descriptor set {1}", m_Name, name);
			return nullptr;
		}
		return &m_WriteDescriptorSets.at(name);
	}

}