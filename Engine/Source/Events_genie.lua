PsyProjectEngineLib( "Engine_Events" )
  configuration "*"
  	files {
      "./Shared/Events/**.h", 
      "./Shared/Events/**.inl", 
      "./Shared/Events/**.cpp", 
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
   		}

   	configuration "vs2012"
   		links {
   		}
