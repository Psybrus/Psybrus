dxsdkpath = os.getenv("DXSDK_DIR")
dxversion = "(June 2010)"
if (dxsdkpath == nil or string.find(dxsdkpath, dxversion) == nil) then
   print ("Please install DirectX SDK" .. dxversion)
   os.exit()
else
   print ("Got DirectX SDK Path: " .. dxsdkpath)
end

project "LD25Game"
	kind "WindowedApp"
	language "C++"
	files { "./Source/**.h", "./Source/**.c", "./Source/**.cpp" }
	includedirs {
		"./Source/",
		"../Engine/Source/Shared/",
		"../Engine/Source/Platforms/Windows/"
	}

	-- External includes.
	includedirs { "../External/jsoncpp/include" }
	includedirs { "../External/portaudio/include" }

   -- DirectX
   configuration "windows"
      libdirs { dxsdkpath .. "/Lib/x86" }
      links {
         "d3d9",
         "d3dx9",
         "dxguid",
         "dsound",
         "dinput8",
         "xinput",
         "msacm32"
      }

   -- Normal windows config.
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
			"External_ogg",
         	"External_png",
         	"External_rapidxml-1.13",
         	"External_portaudio",
         	"External_pcre",
         	"External_mongoose",
			"External_squish",
			"External_tremor",
			"External_zlib",

			-- Engine libs.
			"Engine_Shared",
			"Engine_Windows",
		}
