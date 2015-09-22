PsyProjectEngineLib( "System_Content" )
  configuration "*"
  	files {
      "./Shared/System/Content/**.h", 
      "./Shared/System/Content/**.inl", 
      "./Shared/System/Content/**.cpp", 
    }

  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      BOOST_INCLUDE_PATH,
    }

  	PsyAddEngineLinks {
      "System",
      "System_File",
  	}

  configuration "linux-*"
      files {
          "./Platforms/Linux/System/Content/*.h", 
          "./Platforms/Linux/System/Content/*.inl", 
          "./Platforms/Linux/System/Content/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "osx-*"
      files {
          "./Platforms/OSX/System/Content/*.h", 
          "./Platforms/OSX/System/Content/*.inl", 
          "./Platforms/OSX/System/Content/*.cpp", 
          "./Platforms/OSX/System/Content/*.mm", 
      }
      includedirs {
          "./Platforms/OSX/",
      }

  configuration( "windows-* or winphone-*" )
        files {
            "./Platforms/Windows/System/Content/*.h", 
            "./Platforms/Windows/System/Content/*.inl", 
            "./Platforms/Windows/System/Content/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

      libdirs {
           BOOST_LIB_PATH
        }

