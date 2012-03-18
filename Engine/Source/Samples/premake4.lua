project "Engine_Samples"
	kind "WindowedApp"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp" }
	includedirs { "./", "../Shared/", "../Platforms/Windows/" }

	-- External includes.
	includedirs { "../../../External/jsoncpp/include" }
	includedirs { "../../../External/zlib" } -- TODO: Fix this properly.

	configuration "windows"
   		links {
   			-- Windows libs.
   			"user32",
   			"gdi32",
   			"opengl32",
			   "../../External/openal/libs/Win32/OpenAL32", -- Relative path to project.
   			"winmm",

   			-- External libs.
   			"External_Box2D_v2.2.1",
   			"External_enet",
   			"External_freetype",
   			"External_glew",
   			"External_jsoncpp",
   			"External_libb64",
   			"External_ogg",
   			"External_png",
   			"External_squish",
   			"External_tremor",
   			"External_zlib",

   			-- Engine libs.
   			"Engine_Shared",
   			"Engine_Windows",
   		}
