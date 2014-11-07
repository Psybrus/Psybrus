PsyProjectEngineLib( "System_Sound" )
  configuration "*"
  	files {
      "./Shared/System/Sound/**.h", 
      "./Shared/System/Sound/**.inl", 
      "./Shared/System/Sound/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/SoLoud/include/",
      BOOST_INCLUDE_PATH,
    }

  	PsyAddEngineLinks {
      "System"
    }

    PsyAddExternalLinks {
      "jsoncpp",
      "libb64",
      "SoLoud"
  	}

  configuration "linux-*"
      files {
      }

      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows-*"
      files {
      }
      
      includedirs {
            "./Platforms/Windows/",
      }

      libdirs {
           BOOST_LIB_PATH
      }

