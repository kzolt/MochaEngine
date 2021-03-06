#pragma once

#include <glm/glm.hpp>

#include "Mocha/Renderer/RendererAPI.h"

namespace Mocha {

	enum class FramebufferFormat
	{
		None	= 0,
		RGBA8	= 1,
		RGBA16F	= 2
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		glm::vec4 ClearColor;
		FramebufferFormat Format = FramebufferFormat::RGBA8;
		uint32_t Samples = 1; // Multisampling

		bool SwapchainTarget = false;
	};

	class Framebuffer : public RefCounted
	{
	public:
		virtual ~Framebuffer() {}
		virtual void Bind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) = 0;

		virtual void BindTexture(uint32_t slot = 0) const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	class FramebufferPool final
	{
	public:
		FramebufferPool(uint32_t maxFBs = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(const Ref<Framebuffer>& framebuffer);

		std::vector<Ref<Framebuffer>>& GetAll() { return m_Pool; }
		const std::vector<Ref<Framebuffer>>& GetAll() const { return m_Pool; }

		inline static FramebufferPool* GetGlobal() { return s_Instance; }
	private:
		std::vector<Ref<Framebuffer>> m_Pool;

		static FramebufferPool* s_Instance;
	};

}