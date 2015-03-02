PsyProjectEngineLib( "Reflection" )
  configuration "*"
  	files {
      "./Shared/Reflection/**.h", 
      "./Shared/Reflection/**.inl",
      "./Shared/Reflection/**.cpp",
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
    "Engine_Base",
  }

  PsyAddExternalLinks {
    "libb64",
  }
