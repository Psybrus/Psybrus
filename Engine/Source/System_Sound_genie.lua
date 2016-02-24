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
      "../../External/SoLoud/include/"
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

  configuration "osx-*"
      files {
      }

      includedirs {
          "./Platforms/OSX/",
      }

  configuration "windows-* or winphone-*"
      files {
      }
      
      includedirs {
            "./Platforms/Windows/",
      }

