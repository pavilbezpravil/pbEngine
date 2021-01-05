project "imgui_node_editor"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{ 
		"**.h", 
		"**.inl", 
		"**.cpp", 
	}

	includedirs
	{
		-- "pbe/vendor/imgui-node-editor",
		"pbe/vendor/ImGui",
		"../ImGui",
	}

	-- links 
	-- { 
	-- 	"pbe/vendor/ImGui",
	-- }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
