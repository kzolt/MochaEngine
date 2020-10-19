#pragma once

#include "Mocha/Core/Base.h"
#include "Mocha/Core/Window.h"

namespace Mocha {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		Ref<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined inside client
	Application* CreateApplication();
}