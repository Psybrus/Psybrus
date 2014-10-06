project "Engine_Serialisation"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Serialisation/**.h", 
    "./Shared/Serialisation/**.inl",
    "./Shared/Serialisation/**.cpp",
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    "../../External/pcre/",
    "../../External/zlib/",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    boostInclude,
  }

  print( "Serialisation" .. psybrusSDK )

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
   		}

   	configuration "vs2012"
   		links {
   		}
