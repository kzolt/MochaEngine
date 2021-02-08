#pragma once

#include "Mocha/Core/Layer.h"

namespace Mocha {

	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		static ImGuiLayer* Create();
	};

}