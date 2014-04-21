project "Engine_Import"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Import/**.h", 
    "./Shared/Import/**.inl", 
    "./Shared/Import/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/pcre/",
    psybrusSDK .. "/External/zlib/",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
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
        "Engine_Reflection",

   			-- External libs.
   			"External_pcre",
   			"External_zlib",
        "External_jsoncpp",
        "External_libb64",
        "External_ogg",
        "External_png",
        "External_squish",
        "External_tremor",
   		}

   	configuration "vs2012"
   		links {
   		}
