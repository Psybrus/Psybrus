project "Engine_System_Debug"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Debug/**.h", 
    "./Shared/System/Debug/**.inl", 
    "./Shared/System/Debug/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    "../../External/webby/",
    boostInclude,
  }

 		links {
      -- Engine libs.
      "Engine_System",
      "Engine_System_Content",

 			-- External libs.
      "External_jsoncpp",
      "External_libb64",
      "External_webby",
 		}

  configuration "linux"
      files {
          "./Platforms/Linux/System/Debug/*.h", 
          "./Platforms/Linux/System/Debug/*.inl", 
          "./Platforms/Linux/System/Debug/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
        files {
            "./Platforms/Windows/System/Debug/*.h", 
            "./Platforms/Windows/System/Debug/*.inl", 
            "./Platforms/Windows/System/Debug/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

      libdirs {
           boostLib
        }
