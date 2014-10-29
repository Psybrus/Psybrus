PsyProjectEngineLib( "Engine_Import" )
  configuration "*"
  	files {
      "./Shared/Import/**.h", 
      "./Shared/Import/**.inl", 
      "./Shared/Import/**.cpp", 
    }
    
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/pcre/",
      "../../External/zlib/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/ogg/include/",
      "../../External/png/",
      "../../External/squish/",
      "../../External/tremor/",
      boostInclude,
    }

    libdirs {
       boostLib
    }

  	links {
    -- Engine libs.
      "Engine_Base",
      "Engine_Math",
      "Engine_Reflection",

  		-- External libs.
  		"External_zlib",
      "External_ogg",
      "External_png",
      "External_squish",
      "External_tremor",
  	}

