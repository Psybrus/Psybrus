PsyProjectEngineLib( "System_Os" )
  configuration "*"
    files {
      "./Shared/System/Os/*.h", 
      "./Shared/System/Os/*.inl", 
      "./Shared/System/Os/*.cpp", 
    }
    includedirs {
      "./Shared/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
    }

    PsyAddEngineLinks {
      -- Engine libs.
      "System",
    }

  configuration "linux-*"
    defines { 
      "OS_USE_SDL=1",
    }
    files {
      "./Shared/System/Os/SDL/*.h", 
      "./Shared/System/Os/SDL/*.inl", 
      "./Shared/System/Os/SDL/*.cpp", 
      "./Platforms/Linux/System/Os/*.h", 
      "./Platforms/Linux/System/Os/*.inl", 
      "./Platforms/Linux/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Linux/",
      "../../External/ThinkGear/",
    }

  configuration "osx-*"
    defines { 
      "OS_USE_SDL=1",
    }
    files {
      "./Shared/System/Os/SDL/*.h", 
      "./Shared/System/Os/SDL/*.inl", 
      "./Shared/System/Os/SDL/*.cpp", 
      "./Platforms/OSX/System/Os/*.h", 
      "./Platforms/OSX/System/Os/*.inl", 
      "./Platforms/OSX/System/Os/*.cpp", 
      "./Platforms/OSX/System/Os/*.mm", 
    }
    includedirs {
      "./Platforms/OSX/",
      "../../External/SDL-mirror/include/",
    }

  configuration "android-*"
    defines { 
      "OS_USE_SDL=0",
    }
    files {
      "./Platforms/Android/System/Os/*.h", 
      "./Platforms/Android/System/Os/*.inl", 
      "./Platforms/Android/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Android/",
    }
    
  configuration "html5-clang-asmjs"
    defines { 
      "OS_USE_SDL=1",
      "LOCAL_SDL_LIBRARY=1",
    }
    files {
      "./Shared/System/Os/SDL/*.h", 
      "./Shared/System/Os/SDL/*.inl", 
      "./Shared/System/Os/SDL/*.cpp", 
      --"./Platforms/HTML5/System/Os/*.h", 
      --"./Platforms/HTML5/System/Os/*.inl", 
      --"./Platforms/HTML5/System/Os/*.cpp", 
    }
    includedirs {
      --"./Platforms/HTML5/",
      "../../External/SDL-mirror/include/",
    }

  configuration "windows-*"
    defines { 
      "OS_USE_SDL=1",
      "LOCAL_SDL_LIBRARY=1",
    }
    files {
      "./Shared/System/Os/SDL/*.h", 
      "./Shared/System/Os/SDL/*.inl", 
      "./Shared/System/Os/SDL/*.cpp", 
      "./Platforms/Windows/System/Os/*.h", 
      "./Platforms/Windows/System/Os/*.inl", 
      "./Platforms/Windows/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Windows/",
      "../../External/SDL-mirror/include/",
    }

  configuration "winphone-*"
    includedirs {
      "./Platforms/Windows/",
    }
