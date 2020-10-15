#pragma once

#ifdef MC_PLATFORM_WINDOWS

extern Mocha::Application* Mocha::CreateApplication();

int main(int argc, char** argv)
{
	Mocha::Log::Init();

	auto app = Mocha::CreateApplication();
	app->Run();
	delete app;
}

#endif