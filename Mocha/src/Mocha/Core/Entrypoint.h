#pragma once

#ifdef MC_PLATFORM_WINDOWS

extern Mocha::Application* Mocha::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Mocha::CreateApplication();
	app->Run();
	delete app;
}

#endif