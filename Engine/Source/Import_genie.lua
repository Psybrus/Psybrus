PsyProjectEngineLib( "Import" )
  configuration "*"
  	files {
      "./Shared/Import/**.h", 
      "./Shared/Import/**.inl", 
      "./Shared/Import/**.cpp", 
    }
    
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/zlib/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/ogg/include/",
      "../../External/png/",
      "../../External/rg-etc1/",
      "../../External/squish/",
      "../../External/tremor/",
      BOOST_INCLUDE_PATH,
    }

    libdirs {
       BOOST_LIB_PATH
    }

  	PsyAddEngineLinks {
      "Base",
      "Math",
      "Reflection",
    }

    PsyAddExternalLinks {
  		"zlib",
      "png",
      "squish",
  	}
