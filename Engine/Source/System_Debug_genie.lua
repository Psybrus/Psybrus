PsyProjectEngineLib( "System_Debug" )
  configuration "*"
  	files {
      "./Shared/System/Debug/**.h", 
      "./Shared/System/Debug/**.inl", 
      "./Shared/System/Debug/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/RakNet/Source",
      "../../External/rapidxml/"
    }

    PsyAddEngineLinks {
      "System",
      "System_Content",
    }

    PsyAddExternalLinks {
      "jsoncpp",
      "libb64",
  	  "raknet", 
 		}

  defines { "RMT_USE_OPENGL=1" }

  configuration { "windows-* or linux-* or osx-* or android-*" }
    PsyAddExternalLinks { "remotery" }
    includedirs { "../../External/Remotery/lib" }

  configuration { "windows-* or linux-* or osx-* or android-*" }
    PsyAddExternalLinks { "webby" }    
    includedirs { "../../External/webby/" }

  configuration "linux-*"
      files {
          "./Platforms/Linux/System/Debug/*.h", 
          "./Platforms/Linux/System/Debug/*.inl", 
          "./Platforms/Linux/System/Debug/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "osx-*"
      files {
          "./Platforms/OSX/System/Debug/*.h", 
          "./Platforms/OSX/System/Debug/*.inl", 
          "./Platforms/OSX/System/Debug/*.cpp", 
          "./Platforms/OSX/System/Debug/*.mm", 
      }
      includedirs {
          "./Platforms/OSX/",
      }

  configuration "windows-* or winphone-*"
        files {
            "./Platforms/Windows/System/Debug/*.h", 
            "./Platforms/Windows/System/Debug/*.inl", 
            "./Platforms/Windows/System/Debug/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

  configuration "android-*"
        defines {
            "RMT_ENABLED=0",
        }
