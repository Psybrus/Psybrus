PsyProjectEngineLib( "Engine_System_Scene" )
  configuration "*"
  	files {
      "./Shared/System/Scene/**.h", 
      "./Shared/System/Scene/**.inl", 
      "./Shared/System/Scene/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/assimp/include",
      "../../External/bullet/src",
      "../../External/freetype/include",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/HLSLCrossCompiler/include/",
      BOOST_INCLUDE_PATH,
    }

    -- GLEW config.
    defines { "GLEW_STATIC" }

    libdirs {
         BOOST_LIB_PATH
    }

		links {
    -- Engine libs.
    "Engine_System",
    "Engine_System_Content",
    "Engine_System_Debug",
    "Engine_System_File",
    "Engine_System_Network",
    "Engine_System_Os",
    "Engine_System_Renderer",
    "Engine_System_Sound",

			-- External libs.
    "External_assimp",
    "External_BulletPhysics",
    "External_freetype",
    "External_jsoncpp",
    "External_HLSLCrossCompiler"
		}

  configuration "linux-*"
      files {
      }

      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows-*"
      files {
      }
      
      includedirs {
            "./Platforms/Windows/",
      }

      libdirs {
           BOOST_LIB_PATH
      }
