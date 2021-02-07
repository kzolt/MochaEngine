#pragma once

#include "Mocha/Core/Base.h"

#include "Mocha/Renderer/VertexBuffer.h"
#include "Mocha/Renderer/Shader.h"
#include "Mocha/Renderer/RenderPass.h"

namespace Mocha {

	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		VertexBufferLayout Layout;
		Ref<RenderPass> RenderPass;
	};

	class Pipeline : public RefCounted
	{
	public:
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;

		virtual void Bind() = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};

}