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
IncludeDir["ImGui"] = "Mocha/vendor/ImGui"
IncludeDir["STB"] = "Mocha/vendor/stb/include"

LibraryDir = {}
LibraryDir["shaderc"] = "vendor/shaderc/lib/Debug/shaderc.lib"
LibraryDir["shaderc_util"] = "vendor/shaderc/lib/Debug/shaderc_util.lib"
LibraryDir["glslang"] = "vendor/glslang/lib/Debug/glslangd.lib"
LibraryDir["glslang_MachineIndependent"] = "vendor/glslang/lib/Debug/MachineIndependentd.lib"
LibraryDir["glslang_SPIRV"] = "vendor/glslang/lib/Debug/SPIRVd.lib"
LibraryDir["glslang_OGLCompiler"] = "vendor/glslang/lib/Debug/OGLCompilerd.lib"
LibraryDir["glslang_OSDependent"] = "vendor/glslang/lib/Debug/OSDependentd.lib"
LibraryDir["glslang_GenericCodeGen"] = "vendor/glslang/lib/Debug/GenericCodeGend.lib"
LibraryDir["SPIRV_Tools"] = "vendor/SPIRV-Tools/lib/Debug/SPIRV-Tools.lib"
LibraryDir["SPIRV_Tools_opt"] = "vendor/SPIRV-Tools/lib/Debug/SPIRV-Tools-opt.lib"
LibraryDir["Vulkan"] = "vendor/Vulkan/lib/vulkan-1.lib"

group "Dependencies"
include "Mocha/vendor/GLFW"
include "Mocha/vendor/ImGui"
include "Mocha/vendor"
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
		"%{prj.name}/src/**.cpp",

		"%{IncludeDir.ImGui}/examples/imgui_impl_vulkan.cpp",
        "%{IncludeDir.ImGui}/examples/imgui_impl_glfw.cpp",

		"Mocha/Vendor/stb/stb_image.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.STB}"
	}

	links
	{
		"GLFW",
		"ImGui",
		"SPIRV-Cross",
		"%{LibraryDir.Vulkan}",

		"%{LibraryDir.shaderc}",
		"%{LibraryDir.shaderc_util}",
		"%{LibraryDir.glslang}",
		"%{LibraryDir.glslang_MachineIndependent}",
		"%{LibraryDir.glslang_SPIRV}",
		"%{LibraryDir.glslang_OGLCompiler}",
		"%{LibraryDir.glslang_OSDependent}",
		"%{LibraryDir.glslang_GenericCodeGen}",
		"%{LibraryDir.SPIRV_Tools}",
		"%{LibraryDir.SPIRV_Tools_opt}"
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
		"Mocha/vendor",
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