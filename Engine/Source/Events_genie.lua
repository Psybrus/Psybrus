PsyProjectEngineLib( "Events" )
  configuration "*"
  	files {
      "./Shared/Events/**.h", 
      "./Shared/Events/**.inl", 
      "./Shared/Events/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/zlib/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      BOOST_INCLUDE_PATH,
    }

	configuration "windows-*"
	    libdirs {
           BOOST_LIB_PATH
        }

   		PsyAddEngineLinks {
        "Base",
   		}
