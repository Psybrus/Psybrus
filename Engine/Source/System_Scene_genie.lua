PsyProjectEngineLib( "System_Scene" )
  configuration "*"
  	files {
      "./Shared/System/Scene/**.h", 
      "./Shared/System/Scene/**.inl", 
      "./Shared/System/Scene/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "../../External/assimp/include",
      "../../External/bullet3/Extras/HACD",
      "../../External/bullet3/src",
      "../../External/freetype/include",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/HLSLCrossCompiler/include/",
      "../../External/hlsl2glslfork/include/",
      "../../External/hlsl2glslfork/include/",
      "../../External/glslang/glslang/Public",
      "../../External/glsl-optimizer/src",
      "../../External/rapidxml/",
      BOOST_INCLUDE_PATH,
    }

    -- GLEW config.
    defines { "GLEW_STATIC" }

    libdirs {
         BOOST_LIB_PATH
    }

		PsyAddEngineLinks {
      "System",
      "System_Content",
      "System_Debug",
      "System_File",
      "System_Network",
      "System_Os",
      "System_Renderer",
      "System_Sound",
    }

    PsyAddExternalLinks {
      "assimp",
      "BulletPhysics",
      "freetype",
      "jsoncpp",
      "glsl-optimizer",
      "hlsl2glslfork",
      "HLSLCrossCompiler"
		}

  configuration "linux-*"
      files {
      }

      includedirs {
          "./Platforms/Linux/",
      }

  configuration "osx-*"
      files {
      }

      includedirs {
          "./Platforms/OSX/",
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
