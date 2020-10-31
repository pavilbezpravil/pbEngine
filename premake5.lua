workspace "pbe"
	architecture "x64"
	targetdir "build"
	
	configurations 
	{ 
		"Debug", 
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	startproject "pbeEditor"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "pbe/vendor/GLFW/include"
IncludeDir["ImGui"] = "pbe/vendor/ImGui"
IncludeDir["glm"] = "pbe/vendor/glm"
IncludeDir["Box2D"] = "pbe/vendor/Box2D/include"
IncludeDir["entt"] = "pbe/vendor/entt/single_include"
IncludeDir["FastNoise"] = "pbe/vendor/FastNoise/Cpp"
IncludeDir["mono"] = "pbe/vendor/mono/include"
IncludeDir["spdlog"] = "pbe/vendor/spdlog/include"
IncludeDir["WinPixEventRuntime"] = "pbe/vendor/WinPixEventRuntime/Include/WinPixEventRuntime"

LibraryDir = {}
LibraryDir["mono"] = "vendor/mono/lib/Debug/mono-2.0-sgen.lib"
LibraryDir["WinPixEventRuntime"] = "vendor/WinPixEventRuntime/bin/WinPixEventRuntime.lib"

group "Dependencies"
include "pbe/vendor/GLFW"
include "pbe/vendor/ImGui"
include "pbe/vendor/Box2D"
group ""

group "Core"
project "pbe"
	location "pbe"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "pbe/src/pch.cpp"

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp",

		"%{prj.name}/vendor/FastNoise/**.cpp",

		"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
		"%{prj.name}/vendor/yaml-cpp/src/**.h",
		"%{prj.name}/vendor/yaml-cpp/include/**.h"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.WinPixEventRuntime}",
		"%{IncludeDir.FastNoise}",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/stb/include",
		"%{prj.name}/vendor/yaml-cpp/include"
	}
	
	links 
	{ 
		"GLFW",
		"ImGui",
		"Box2D",
		"d3d12",
		"dxgi",
		"%{LibraryDir.mono}",
		"%{LibraryDir.WinPixEventRuntime}",
	}
	
	filter "files:pbe/vendor/FastNoise/**.cpp or files:pbe/vendor/yaml-cpp/src/**.cpp"
   	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		
		defines 
		{ 
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL"
		}

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		symbols "On"
				
	filter "configurations:Release"
		defines "HZ_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DIST"
		optimize "On"

project "pbe-ScriptCore"
	location "pbe-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.cs", 
	}
group ""

group "Tools"
project "pbeEditor"
	location "pbeEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
		"pbe"
	}
	
	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp" 
	}
	
	includedirs 
	{
		"%{prj.name}/src",
		"pbe/src",
		"pbe/vendor",
		"%{IncludeDir.entt}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}"
	}

	postbuildcommands 
	{
		'{COPY} "../pbeEditor/assets" "%{cfg.targetdir}/assets"',
		'{COPY} "pbe/vendor/WinPixEventRuntime/bin" "%{cfg.targetdir}"'
	}
	
	filter "system:windows"
		systemversion "latest"
				
		defines 
		{ 
			"HZ_PLATFORM_WINDOWS"
		}
	
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		symbols "on"

		links
		{
			"pbe/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../pbe/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../pbe/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}
				
	filter "configurations:Release"
		defines "HZ_RELEASE"
		optimize "on"

		links
		{
			"pbe/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../pbe/vendor/assimp/bin/Release/assimp-vc141-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../pbe/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Dist"
		defines "HZ_DIST"
		optimize "on"

		links
		{
			"pbe/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../pbe/vendor/assimp/bin/Release/assimp-vc141-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../pbe/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}
group ""

project "pbe-ScriptCore"
	location "pbe-ScriptCore"
	kind "SharedLib"
	language "C#"
	systemversion("latest")

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.cs", 
	}

group ""