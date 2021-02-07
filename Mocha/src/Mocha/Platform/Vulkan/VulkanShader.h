#pragma once

#include "Mocha/Renderer/Shader.h"
#include "Vulkan.h"

namespace Mocha {

	class VulkanShader : public Shader
	{
	public:
		struct UniformBuffer
		{
			VkDeviceMemory Memory;
			VkBuffer Buffer;
			VkDescriptorBufferInfo Descriptor;
			uint32_t Size = 0;
			uint32_t BindingPoint = 0;
			std::string Name;
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct ImageSampler
		{
			uint32_t BindingPoint = 0;
			std::string Name;
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct PushConstantRange
		{
			VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t Offset = 0;
			uint32_t Size = 0;
		};

	public:
		VulkanShader(const std::string& path);
		virtual ~VulkanShader();

		virtual void Reload() override;
		virtual void Bind() override;

		virtual inline RendererID GetRendererID() const override;

		virtual void SetUniformBuffer(const std::string& name, const void* data, uint32_t size) override;
		virtual void SetUniform(const std::string& name, float value) override;
		virtual void SetUniform(const std::string& name, int value) override;
		virtual void SetUniform(const std::string& name, const glm::vec2& value) override;
		virtual void SetUniform(const std::string& name, const glm::vec3& value) override;
		virtual void SetUniform(const std::string& name, const glm::vec4& value) override;
		virtual void SetUniform(const std::string& name, const glm::mat3& value) override;
		virtual void SetUniform(const std::string& name, const glm::mat4& value) override;

		virtual inline const std::string& GetName() const override { return m_Name; }
		virtual inline const std::unordered_map<std::string, ShaderBuffer>& GetShaderBuffers() const override { return m_Buffers; }
		virtual inline const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResources() const override { return {}; }

		virtual inline void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) override;

		// Vulkan-specific
		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const { return m_ShaderStages; }

		void* MapUniformBuffer(uint32_t bindingPoint);
		void UnmapUniformBuffer(uint32_t bindingPoint);

		VkDescriptorSet GetDescriptorSet() { return m_DescriptorSet; }
		VkDescriptorSetLayout GetDescriptorSetLayout() { return m_DescriptorSetLayout; }

		UniformBuffer& GetUniformBuffer(uint32_t binding = 0) { MC_CORE_ASSERT(m_UniformBuffers.size() > 0); return m_UniformBuffers[binding]; }
		const std::vector<PushConstantRange>& GetPushConstantRanges() const { return m_PushConstantRanges; }

		VkDescriptorSet CreateDescriptorSet();
		const VkWriteDescriptorSet* GetDescriptorSet(const std::string& name) const;

	private:
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);
		void CommpileOrGetVulkanBinary(std::array<std::vector<uint32_t>, 2>& outputBinary, bool forceCompile);
		void LoadAndCreateVertexShader(VkPipelineShaderStageCreateInfo& shaderStage, const std::vector<uint32_t>& shaderData);
		void LoadAndCreateFragmentShader(VkPipelineShaderStageCreateInfo& shaderStage, const std::vector<uint32_t>& shaderData);
		void Reflect(VkShaderStageFlagBits shaderStage, const std::vector<uint32_t>& shaderData);

		void CreateDescriptors();

		void AllocateUniformBuffer(UniformBuffer& dst);

	private:
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::unordered_map<VkShaderStageFlagBits, std::string> m_ShaderSource;

		std::string m_AssetPath;
		std::string m_Name;

		std::unordered_map<uint32_t, UniformBuffer> m_UniformBuffers;
		std::unordered_map<uint32_t, ImageSampler> m_ImageSamplers;
		std::unordered_map<std::string, VkWriteDescriptorSet> m_WriteDescriptorSets;
		std::vector<PushConstantRange> m_PushConstantRanges;

		std::unordered_map<std::string, ShaderBuffer> m_Buffers;

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorSet m_DescriptorSet;
		VkDescriptorPool m_DescriptorPool;
	};

}