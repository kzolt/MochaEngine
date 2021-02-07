#pragma once

#include <string>
#include <functional>

#include "Mocha/Core/Base.h"
#include "Mocha/Core/Events/Event.h"

#include "Mocha/Renderer/RendererContext.h"

namespace Mocha {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool VSync;

		WindowProps(const std::string& title = "Mocha Engine", uint32_t width = 1280, uint32_t height = 720)
			: Title(std::string(title)), Width(width), Height(height), VSync(true) {}
	};

	// Interface representing a desktop system based Window
	class Window : public RefCounted
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void ProcessEvent() = 0;
		virtual void SwapBuffers() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual Ref<RendererContext> GetRenderContext() = 0;

		static Window* Create(const WindowProps& = WindowProps());
	};

}