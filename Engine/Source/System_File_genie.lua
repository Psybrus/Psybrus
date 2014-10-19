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
          "./Platforms/Linux/System/File/*.h", 
          "./Platforms/Linux/System/File/*.inl", 
          "./Platforms/Linux/System/File/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
        files {
            "./Platforms/Windows/System/File/*.h", 
            "./Platforms/Windows/System/File/*.inl", 
            "./Platforms/Windows/System/File/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

      libdirs {
           boostLib
        }
