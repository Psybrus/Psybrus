project "Engine_Reflection"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Reflection/**.h", 
    "./Shared/Reflection/**.inl",
    "./Shared/Reflection/**.cpp",
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

   			-- External libs.
        "External_libb64",
   		}

   	configuration "vs2012"
   		links {
   		}
