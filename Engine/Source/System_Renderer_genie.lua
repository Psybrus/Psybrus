PsyProjectEngineLib( "System_Renderer" )
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

 		PsyAddEngineLinks {
      "System",
    }

    PsyAddExternalLinks {
      "jsoncpp",
      "libb64",
 		}

  -- Windows and linux get glew.
  configuration { "windows-* or linux-*" }
    defines { "GLEW_STATIC" }
    includedirs {
      "../../External/glew/include",
    }

    PsyAddExternalLinks {
      "glew",
    }

  configuration "*"
      files {
          "./Shared/System/Renderer/Null/*.h", 
          "./Shared/System/Renderer/Null/*.inl", 
          "./Shared/System/Renderer/Null/*.cpp", 
      }

  configuration "linux-*"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "android-*"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/Android/",
      }

  configuration "asmjs"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/HTML5/",
      }

  configuration "windows-*"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
          "./Shared/System/Renderer/D3D11/*.h", 
          "./Shared/System/Renderer/D3D11/*.inl", 
          "./Shared/System/Renderer/D3D11/*.cpp", 
          "./Shared/System/Renderer/D3D12/*.h", 
          "./Shared/System/Renderer/D3D12/*.inl", 
          "./Shared/System/Renderer/D3D12/*.cpp", 
          "./Shared/System/Renderer/D3D12/Shaders/*.hlsl",
          "./Shared/System/Renderer/D3D12/Shaders/*.h",
      }
      includedirs {
            "./Platforms/Windows/",
      }

      libdirs {
           BOOST_LIB_PATH
      }
