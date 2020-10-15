#pragma once

namespace Mocha {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined inside client
	Application* CreateApplication();
}