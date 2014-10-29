PsyProjectEngineLib( "Engine_Serialisation" )
  configuration "*"
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
      BOOST_INCLUDE_PATH,
    }

	configuration "windows"
	    libdirs {
           BOOST_LIB_PATH
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
