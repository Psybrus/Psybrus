project "MiniLD33Game"
	kind "WindowedApp"
	language "C++"
	files { "./Source/**.h", "./Source/**.c", "./Source/**.cpp" }
	includedirs { "./Source/", "../Engine/Source/Shared/", "../Engine/Source/Platforms/Windows/", }

	-- External includes.
	includedirs {
      "../External/enet/include",
      "../External/jsoncpp/include",
      "../External/libircclient-1.6/include",
      "../External/libb64/include",
      "../External/stund",
      "../External/zlib"
   }

	configuration "windows"
   		links {
   			-- Windows libs.
   			"user32",
   			"gdi32",
   			"opengl32",
   			"winmm",
            "ws2_32",
            "IPHlpApi",
            "../../External/openal/libs/Win32/OpenAL32", -- Relative path to project.

   			-- External libs.
   			"External_enet",
   			"External_freetype",
   			"External_glew",
   			"External_jsoncpp",
   			"External_libb64",
            "External_libircclient-1.6",
   			"External_ogg",
   			"External_png",
   			"External_squish",
            "External_stund",
   			"External_tremor",
   			"External_zlib",

   			-- Engine libs.
   			"Engine_Shared",
   			"Engine_Windows",

            -- Integration libraries.
            -- "Engine_IntegrationGwen",
   		}
