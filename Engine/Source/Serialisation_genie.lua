PsyProjectEngineLib( "Serialisation" )
  configuration "*"
  	files {
      "./Shared/Serialisation/**.h", 
      "./Shared/Serialisation/**.inl",
      "./Shared/Serialisation/**.cpp",
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
    "Reflection",
  }

  PsyAddEngineLinks {
  	"zlib",
    "jsoncpp",
    "libb64",
  }
