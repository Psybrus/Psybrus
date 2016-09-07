PsyProjectEngineLib( "Editor" )
  configuration "*"
  	files {
      "./Shared/Editor/**.h", 
      "./Shared/Editor/**.inl", 
      "./Shared/Editor/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
    }

		PsyAddEngineLinks {
      "System",
      "System_Content",
      "System_Debug",
      "System_File",
      "System_Network",
      "System_Os",
      "System_Renderer",
      "System_Sound",
      "System_Scene",
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
