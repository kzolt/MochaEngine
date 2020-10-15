#include <Mocha.h>

class Sandbox : public Mocha::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{

	}
};

Mocha::Application* Mocha::CreateApplication()
{
	return new Sandbox();
}