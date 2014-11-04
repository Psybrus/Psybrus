PsyProjectEngineLib( "Engine_System_File" )
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

    links {
      -- Engine libs.
      "Engine_System",
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
