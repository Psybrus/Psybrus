project "MiniLD33Game"
	kind "WindowedApp"
	language "C++"
	files { "./Source/**.h", "./Source/**.c", "./Source/**.cpp" }
	includedirs { "./Source/", "../Engine/Source/Shared/", "../Engine/Source/Platforms/Windows/", }

	-- External includes.
	includedirs {
      "../External/zlib",
      "../External/jsoncpp/include",
      "../External/libircclient-1.6/include",
      "../External/libb64/include"
   }

	configuration "windows"
   		links {
   			-- Windows libs.
   			"user32",
   			"gdi32",
   			"opengl32",
   			"winmm",
            "../../External/openal/libs/Win32/OpenAL32", -- Relative path to project.
            "../../External/libircclient-1.6/bin/libircclient", -- Relative path to project.

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

            -- Integration libraries.
            "Engine_IntegrationGwen",
   		}
