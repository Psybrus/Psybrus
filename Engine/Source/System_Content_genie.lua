PsyProjectEngineLib( "Engine_System_Content" )
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

  	links {
      -- Engine libs.
      "Engine_System",
      "Engine_System_File",
  		}

  configuration "linux"
      files {
          "./Platforms/Linux/System/Content/*.h", 
          "./Platforms/Linux/System/Content/*.inl", 
          "./Platforms/Linux/System/Content/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
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
