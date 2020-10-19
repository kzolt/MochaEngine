#include "mcpch.h"
#include "Application.h"

namespace Mocha {

	Application::Application()
	{
		m_Window = Window::Create();
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->ProcessEvent();
			m_Window->SwapBuffers();
		}
	}

}