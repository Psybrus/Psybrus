PsyProjectEngineLib( "Engine_System_Sound" )
  configuration "*"
  	files {
      "./Shared/System/Sound/**.h", 
      "./Shared/System/Sound/**.inl", 
      "./Shared/System/Sound/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/SoLoud/include/",
      BOOST_INCLUDE_PATH,
    }

  	links {
      -- Engine libs.
      "Engine_System",

    		-- External libs.
      "External_jsoncpp",
      "External_libb64",
      "External_SoLoud"
  	}

  configuration "linux"
      files {
      }

      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
      files {
      }
      
      includedirs {
            "./Platforms/Windows/",
      }

      libdirs {
           BOOST_LIB_PATH
      }
