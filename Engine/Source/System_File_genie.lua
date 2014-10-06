project "Engine_System_File"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/File/**.h", 
    "./Shared/System/File/**.inl", 
    "./Shared/System/File/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    boostInclude,
  }

	links {
    -- Engine libs.
    "Engine_System",
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
           boostLib
        }
