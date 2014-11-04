PsyProjectEngineLib( "Engine_System_Debug" )
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
      BOOST_INCLUDE_PATH,
    }

 		links {
      -- Engine libs.
      "Engine_System",
      "Engine_System_Content",

 			-- External libs.
      "External_jsoncpp",
      "External_libb64",
 		}

  configuration { "windows or linux-gcc or linux-clang" }
    defines { "USE_WEBBY=1" }
    links { "External_webby" }    
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

  configuration "windows-*"
        files {
            "./Platforms/Windows/System/Debug/*.h", 
            "./Platforms/Windows/System/Debug/*.inl", 
            "./Platforms/Windows/System/Debug/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

      libdirs {
           BOOST_LIB_PATH
        }

