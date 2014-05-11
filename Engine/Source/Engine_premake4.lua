project "Engine"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/*.h", 
    "./Shared/*.inl", 
    "./Shared/*.cpp", 
    "./Platforms/Windows/*.h", 
    "./Platforms/Windows/*.inl", 
    "./Platforms/Windows/*.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/bullet-2.81-rev2613/src",
    psybrusSDK .. "/External/enet/include",
    psybrusSDK .. "/External/freetype/include",
    psybrusSDK .. "/External/glew/include",
    psybrusSDK .. "/External/pcre/",
    psybrusSDK .. "/External/zlib/",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    psybrusSDK .. "/External/mongoose/",
    psybrusSDK .. "/External/openal/include/",
    psybrusSDK .. "/External/ogg/include/",
    psybrusSDK .. "/External/png/",
    psybrusSDK .. "/External/squish/",
    psybrusSDK .. "/External/tremor/",
    boostInclude,
  }

	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_Base",
        "Engine_Events",
        "Engine_Import",
        "Engine_Math",
        "Engine_Reflection",
        "Engine_Serialisation",
        "Engine_System",
        "Engine_System_Content",
        "Engine_System_Debug",
        "Engine_System_File",
        "Engine_System_Network",
        "Engine_System_Os",
        "Engine_System_Renderer",
        "Engine_System_Scene",
        "Engine_System_Sound",
   		}

   	configuration "vs2012"
   		links {
   		}
