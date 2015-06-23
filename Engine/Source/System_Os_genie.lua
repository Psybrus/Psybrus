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
      "../../External/SDL2/include/",
      BOOST_INCLUDE_PATH,
    }

    PsyAddEngineLinks {
      -- Engine libs.
      "System",
    }

  configuration "linux-*"
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

  configuration "android-*"
    files {
      "./Platforms/Android/System/Os/*.h", 
      "./Platforms/Android/System/Os/*.inl", 
      "./Platforms/Android/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Android/",
    }

  configuration "asmjs"
    files {
      "./Platforms/HTML5/System/Os/*.h", 
      "./Platforms/HTML5/System/Os/*.inl", 
    "./Platforms/HTML5/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/HTML5/",
    }

  configuration "windows-*"
    files {
      "./Platforms/Windows/System/Os/*.h", 
      "./Platforms/Windows/System/Os/*.inl", 
      "./Platforms/Windows/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Windows/",
   }

   libdirs {
     BOOST_LIB_PATH
   }
