#pragma once

#include "RendererContext.h"

#include "Mocha/Core/Application.h"

namespace Mocha {

	class Renderer
	{
	public:
		static Ref<RendererContext> GetContext()
		{
			return Application::Get().GetWindow().GetRenderContext();
		}

		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);

	private:

	};

}