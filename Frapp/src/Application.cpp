#include <Mocha.h>
#include <GLFW/glfw3.h>

class Frapp : public Mocha::Application
{
public:
	Frapp()
	{

	}

	~Frapp()
	{

	}

private:

};

Mocha::Application* Mocha::CreateApplication()
{
	return new Frapp();
}