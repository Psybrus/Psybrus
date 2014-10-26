project "Engine_System_Renderer"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Renderer/*.h", 
    "./Shared/System/Renderer/*.inl", 
    "./Shared/System/Renderer/*.cpp", 
  }

	includedirs {
    "./Shared/",
    "../../External/glew/include",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    "../../External/SDL2/include/",
    boostInclude,
  }

  -- GLEW config.
  defines { "GLEW_STATIC" }

 		links {
      -- Engine libs.
      "Engine_System",

 			-- External libs.
      "External_glew",
      "External_jsoncpp",
      "External_libb64",
 		}

  configuration "linux"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
          "./Shared/System/Renderer/D3D11/*.h", 
          "./Shared/System/Renderer/D3D11/*.inl", 
          "./Shared/System/Renderer/D3D11/*.cpp", 
      }
      includedirs {
            "./Platforms/Windows/",
      }

      libdirs {
           boostLib
      }
