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
      "../../External/fcpp",
      "../../External/freetype/include",
      "../../External/glew/include",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/HLSLCrossCompiler/include/",
      "../../External/hlsl2glslfork/include/",
      "../../External/hlsl2glslfork/include/",
      "../../External/glslang/glslang/Public",
      "../../External/glslang/SPIRV",
      "../../External/glsl-optimizer/src",
      "../../External/rapidxml/"
    }

    -- GLEW config.
    defines { "GLEW_STATIC" }

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
      "BulletPhysics",
      "freetype",
      "jsoncpp"
    }

  configuration "linux-* or osx-* or windows-*"
    PsyAddExternalLinks {
      "assimp",
      "fcpp",
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

  configuration "windows-* or winphone-*"
      files {
      }
      
      includedirs {
            "./Platforms/Windows/",
      }
