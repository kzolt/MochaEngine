#include "mcpch.h"
#include "Application.h"

#include "Mocha/Renderer/Framebuffer.h"

#include <imgui.h>

#include <GLFW/glfw3.h>
#include <Windows.h>

#include "Mocha/Platform/Vulkan/VulkanRenderer.h"

#define BIND_EVENT_FN(fn) std::bind(&Application::##fn, this, std::placeholders::_1)

namespace Mocha {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(true);

		m_ImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_ImGuiLayer);

		VulkanRenderer::Init();
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		OnInit();
		while (m_Running)
		{
			m_Window->ProcessEvent();
			
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);

				// TODO: Drawing
				RenderImGui();
				VulkanRenderer::DrawImGui();
				m_ImGuiLayer->End();

				// On Render Thread
				m_Window->GetRenderContext()->BeginFrame();
				m_Window->SwapBuffers();
			}

			float time = GetTime();
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
		}
		OnShutdown();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispacher(event);
		dispacher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
		dispacher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();
		ImGui::Begin("Renderer");
		auto& caps = RendererAPI::GetCapabilities();
		ImGui::Text("Vendor: %s", caps.Vendor.c_str());
		ImGui::Text("Renderer: %s", caps.Renderer.c_str());
		ImGui::Text("Version: %s", caps.Version.c_str());
		ImGui::Text("Frame Time: %.fms\n", m_Timestep.GetMilliseconds());
		ImGui::End();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;

		m_Window->GetRenderContext()->OnResize(width, height);

		auto& fbs = FramebufferPool::GetGlobal()->GetAll();
		for (auto& fb : fbs)
			fb->Resize(width, height);

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}

}