PsyProjectEngineLib( "Engine_System_Renderer" )
  configuration "*"
  	files {
      "./Shared/System/Renderer/*.h", 
      "./Shared/System/Renderer/*.inl", 
      "./Shared/System/Renderer/*.cpp", 
    }

  	includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/SDL2/include/",
      BOOST_INCLUDE_PATH,
    }

 		links {
      -- Engine libs.
      "Engine_System",

 			-- External libs.
      "External_jsoncpp",
      "External_libb64",
 		}

  -- Windows and linux get glew.
  configuration { "windows or linux-gcc or linux-clang" }
    defines { "GLEW_STATIC" }
    includedirs {
      "../../External/glew/include",
    }

    links {
      "External_glew",
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
           BOOST_LIB_PATH
      }
