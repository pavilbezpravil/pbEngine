project "Lua"
	kind "StaticLib"
	language "C"
	-- cppdialect "C++11"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.c",
	}

	includedirs
	{
		"src"
	}

	filter "system:windows"
		systemversion "latest"

	runtime "Release"
	optimize "on"

	-- filter "configurations:Debug"
	-- 	runtime "Debug"
	-- 	symbols "on"

	-- filter "configurations:Release"
	-- 	runtime "Release"
	-- 	optimize "on"

	-- filter "configurations:Dist"
	-- 	runtime "Release"
	-- 	optimize "on"
