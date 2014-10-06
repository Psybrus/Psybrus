project "Engine_System_Content"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Content/**.h", 
    "./Shared/System/Content/**.inl", 
    "./Shared/System/Content/**.cpp", 
  }

	includedirs {
    "./Shared/",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    boostInclude,
  }

  print( "System/Content" .. psybrusSDK )

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
           boostLib
        }
