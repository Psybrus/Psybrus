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
      "../../External/libb64/include/"
    }

 		PsyAddEngineLinks {
      "System",
    }

    PsyAddExternalLinks {
      "jsoncpp",
      "libb64",
 		}

  -- Windows and linux get glew.
  configuration { "windows-* or linux-* or osx-*" }
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

      defines { "WITH_GL=1" }


  configuration "osx-*"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/OSX/",
          "/usr/local/Cellar/sdl2/2.0.3/include" 
      }

      defines { "WITH_GL=1" }

  configuration "android-*"
    files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
    }
    includedirs {
        "./Platforms/Android/",
    }

      defines { "WITH_GL=1" }

  configuration "html5-clang-asmjs"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }
      includedirs {
          "./Platforms/HTML5/",
      }

      defines { "WITH_GL=1" }

  configuration "windows-*"
      files {
          "./Shared/System/Renderer/GL/*.h", 
          "./Shared/System/Renderer/GL/*.inl", 
          "./Shared/System/Renderer/GL/*.cpp", 
      }

      includedirs {
            "./Platforms/Windows/",
      }

      defines { "WITH_GL=1" }



if _OPTIONS["with-dx12"] then
      files {
          "./Shared/System/Renderer/D3D12/*.h", 
          "./Shared/System/Renderer/D3D12/*.inl", 
          "./Shared/System/Renderer/D3D12/*.cpp", 
          "./Shared/System/Renderer/D3D12/Shaders/*.hlsl",
          "./Shared/System/Renderer/D3D12/Shaders/*.h",
      }

      defines { "WITH_DX12=1" }
end

if _OPTIONS["with-vk"] then
      VK_SDK_PATH = os.getenv("VK_SDK_PATH")
      files {
          "./Shared/System/Renderer/VK/*.h", 
          "./Shared/System/Renderer/VK/*.inl", 
          "./Shared/System/Renderer/VK/*.cpp", 
      }

      includedirs {
          VK_SDK_PATH .. "/Include"
      }

      libdirs {
          VK_SDK_PATH .. "/Source/lib"
      }

      links {
          "VKstatic.1",
          "vulkan-1"
      }

      defines { "WITH_VK=1" }
end

  configuration "winphone-*"
      files {
      }

      includedirs {
            "./Platforms/Windows/",
      }
