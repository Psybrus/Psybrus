PsyProjectEngineLib( "System_File" )
  configuration "*"
  	files {
      "./Shared/System/File/**.h", 
      "./Shared/System/File/**.inl", 
      "./Shared/System/File/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      BOOST_INCLUDE_PATH,
    }

    PsyAddEngineLinks {
      -- Engine libs.
      "System",
    }

  configuration "linux-*"
      files {
          "./Platforms/Linux/System/File/*.h", 
          "./Platforms/Linux/System/File/*.inl", 
          "./Platforms/Linux/System/File/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "osx-*"
      files {
          "./Platforms/OSX/System/File/*.h", 
          "./Platforms/OSX/System/File/*.inl", 
          "./Platforms/OSX/System/File/*.cpp", 
          "./Platforms/OSX/System/File/*.mm", 
      }
      includedirs {
          "./Platforms/OSX/",
      }

  configuration "asmjs"
      files {
          "./Platforms/HTML5/System/File/*.h", 
          "./Platforms/HTML5/System/File/*.inl", 
          "./Platforms/HTML5/System/File/*.cpp", 
      }
      includedirs {
          "./Platforms/HTML5/",
      }

  configuration "windows-*"
        files {
            "./Platforms/Windows/System/File/*.h", 
            "./Platforms/Windows/System/File/*.inl", 
            "./Platforms/Windows/System/File/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

      libdirs {
           BOOST_LIB_PATH
        }
