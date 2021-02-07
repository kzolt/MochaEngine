workspace "Mocha"
	architecture "x64"
	startproject "Frapp"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution director)
IncludeDir = {}
IncludeDir["GLFW"] = "Mocha/vendor/GLFW/include"
IncludeDir["Vulkan"] = "Mocha/vendor/Vulkan/include"
IncludeDir["GLM"] = "Mocha/vendor/glm/include/"

LibraryDir = {}
LibraryDir["Vulkan"] = "vendor/Vulkan/lib/vulkan-1.lib"

group "Dependencies"
include "Mocha/vendor/GLFW"
group ""

group "Core"
project "Mocha"
	location "Mocha"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "mcpch.h"
	pchsource "Mocha/src/mcpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.GLM}"
	}

	links
	{
		"GLFW",
		"%{LibraryDir.Vulkan}"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"MC_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "MC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "MC_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "MC_DIST"
		runtime "Release"
		optimize "On"
group ""

group "Tools"
project "Frapp"
	location "Frapp"
	kind "ConsoleApp"
	language "C++"
	staticruntime "on"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.Vulkan}"
	}

	includedirs
	{
		"Mocha/vendor/spdlog/include",
		"Mocha/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLM}"
	}

	links
	{
		"Mocha"
	}

	filter "system:windows"
		systemversion "latest"

	defines
	{
		"MC_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines "MC_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "MC_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "MC_DIST"
		runtime "Release"
		optimize "on"
group ""