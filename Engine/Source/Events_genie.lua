project "Engine_Events"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Events/**.h", 
    "./Shared/Events/**.inl", 
    "./Shared/Events/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/pcre/",
    psybrusSDK .. "/External/zlib/",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    boostInclude,
  }

	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_Base",
   		}

   	configuration "vs2012"
   		links {
   		}
