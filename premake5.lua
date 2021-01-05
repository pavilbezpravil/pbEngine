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
IncludeDir["imgui_node_editor"] = "pbe/vendor/imgui-node-editor"
IncludeDir["glm"] = "pbe/vendor/glm"
IncludeDir["entt"] = "pbe/vendor/entt/single_include"
IncludeDir["FastNoise"] = "pbe/vendor/FastNoise/Cpp"
IncludeDir["spdlog"] = "pbe/vendor/spdlog/include"
IncludeDir["WinPixEventRuntime"] = "pbe/vendor/WinPixEventRuntime/Include/WinPixEventRuntime"
IncludeDir["Lua"] = "pbe/vendor/lua-5.4.1/src"
IncludeDir["Sol2"] = "pbe/vendor/sol2/single/include"
IncludeDir["PhysX"] = "pbe/vendor/PhysX/include"
IncludeDir["pxshared"] = "pbe/vendor/PhysX/pxshared/include"

LibraryDir = {}
LibraryDir["WinPixEventRuntime"] = "pbe/vendor/WinPixEventRuntime/bin"

filter "configurations:Debug"
	LibraryDir["PhysX"] = "pbe/vendor/PhysX/bin/debug"
filter {}

-- filter "configurations:Release"
-- 	LibraryDir["PhysX"] = "pbe/vendor/PhysX/bin/release"
-- filter {}

-- filter "configurations:Dist"
-- 	LibraryDir["PhysX"] = "pbe/vendor/PhysX/bin/release"
-- filter {}

group "Dependencies"
	include "pbe/vendor/GLFW"
	include "pbe/vendor/ImGui"
	include "pbe/vendor/imgui-node-editor"
	include "pbe/vendor/lua-5.4.1"
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
		"%{IncludeDir.imgui_node_editor}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.WinPixEventRuntime}",
		"%{IncludeDir.Lua}",
		"%{IncludeDir.Sol2}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.pxshared}",
		"%{IncludeDir.FastNoise}",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/stb/include",
		"%{prj.name}/vendor/yaml-cpp/include",
	}

	libdirs {
		"%{LibraryDir.WinPixEventRuntime}",
		"%{LibraryDir.PhysX}",
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"imgui_node_editor",
		"Lua",
		"d3d12",
		"dxgi",
		"WinPixEventRuntime",
		-- "LowLevel_static_64",
		-- "LowLevelAABB_static_64",
		-- "LowLevelDynamics_static_64",
		"PhysX_64",
		"PhysXCharacterKinematic_static_64",
		-- "PhysXCommon_64",
		-- "PhysXCooking_64",
		"PhysXExtensions_static_64",
		"PhysXFoundation_64",
		"PhysXPvdSDK_static_64",
	}
	
	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	buildoptions { "/bigobj" }

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
		"%{IncludeDir.ImGui}",
		"pbe/vendor",
		"%{IncludeDir.entt}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.pxshared}",
	}

	print(LibraryDir["WinPixEventRuntime"] )

	postbuildcommands 
	{
		-- '{COPY} "../pbeEditor/assets" "%{cfg.targetdir}/assets"',
		'{COPY} "../%{LibraryDir.WinPixEventRuntime}/WinPixEventRuntime.dll" "%{cfg.targetdir}"',
		'{COPY} "../%{LibraryDir.PhysX}/dll" "%{cfg.targetdir}"',
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
		}
group ""
