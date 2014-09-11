project "Engine_System"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/*.h", 
    "./Shared/System/*.inl", 
    "./Shared/System/*.cpp", 
    "./Platforms/Windows/System/*.h", 
    "./Platforms/Windows/System/*.inl", 
    "./Platforms/Windows/System/*.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/bullet-2.81-rev2613/src",
    psybrusSDK .. "/External/enet/include",
    psybrusSDK .. "/External/freetype/include",
    psybrusSDK .. "/External/glew/include",
    psybrusSDK .. "/External/pcre/",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    psybrusSDK .. "/External/png/",
    psybrusSDK .. "/External/squish/",
    psybrusSDK .. "/External/webby/",
    psybrusSDK .. "/External/zlib/",
    boostInclude,
  }

  -- GLEW config.
  defines { "GLEW_STATIC" }


	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_Base",
        "Engine_Events",
        "Engine_Import",
        "Engine_Reflection",
        "Engine_Math",
        "Engine_Serialisation",
   		}

   	configuration "vs2012"
   		links {
   		}
