#include <Mocha.h>
#include <Mocha/Core/Entrypoint.h>
#include "EditorLayer.h"

class Frapp : public Mocha::Application
{
public:
	Frapp()
	{

	}

	~Frapp()
	{

	}

	virtual void OnInit() override
	{
		PushLayer(new Mocha::EditorLayer());
	}
};

Mocha::Application* Mocha::CreateApplication()
{
	return new Frapp();
}