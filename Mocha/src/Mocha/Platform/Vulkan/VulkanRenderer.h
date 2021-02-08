#pragma once

namespace Mocha {

	class VulkanRenderer
	{
	public:
		// TODO: Work this out 2D/3D or both?
		static void Submit();

		static void OnResize();
		static void Init();
		static void Draw();
		static void DrawImGui();
	};

}