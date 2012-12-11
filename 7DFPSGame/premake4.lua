project "7FPSGame"
	kind "WindowedApp"
	language "C++"
	files { "./Source/**.hh", "./Source/**.h", "./Source/**.c", "./Source/**.cpp" }
	includedirs { "./Source/", "../Engine/Source/Shared/", "../Engine/Source/Platforms/Windows/" }

	-- External includes.
	includedirs { "../External/jsoncpp/include", "../External/portaudio/include" }

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
   			"External_freetype",
   			"External_glew",
   			"External_jsoncpp",
   			"External_libb64",
            "External_mongoose",
   			"External_ogg",
   			"External_png",
            "External_portaudio",
   			"External_squish",
   			"External_tremor",
   			"External_zlib",

   			-- Engine libs.
   			"Engine_Shared",
   			"Engine_Windows",
   		}
