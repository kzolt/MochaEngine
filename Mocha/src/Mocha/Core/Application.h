#pragma once

#include "Mocha/Core/Base.h"
#include "Mocha/Core/Window.h"

#include "Mocha/Core/LayerStack.h"

#include "Mocha/Core/Events/ApplicationEvent.h"

namespace Mocha {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate(Timestep ts) {}

		virtual void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);

		float GetTime() const;

		inline Window& GetWindow() { return *m_Window; }

		static inline Application& Get() { return *s_Instance; }

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true, m_Minimized = false;
		
		LayerStack m_LayerStack;
		Timestep m_Timestep;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// To be defined inside client
	Application* CreateApplication();
}