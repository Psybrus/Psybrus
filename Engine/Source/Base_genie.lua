PsyProjectEngineLib( "Base" )
  configuration "*"
  	files {
      "./Shared/Base/**.h", 
      "./Shared/Base/**.inl", 
      "./Shared/Base/**.cpp",
    }

    includedirs {
      "./Shared/",
      "../../External/zlib/",
      BOOST_INCLUDE_PATH,
    }

    libdirs {
      BOOST_LIB_PATH
    }

    PsyAddExternalLinks {
      "zlib",
    }

  -- Windows
  configuration( "windows-*" )
    files {
      "./Platforms/Windows/Base/**.h",
      "./Platforms/Windows/Base/**.inl",
      "./Platforms/Windows/Base/**.cpp",
    }
    includedirs {
      "./Platforms/Windows/",
    }

  -- Linux
  configuration( "linux-*" )
    files {
      "./Platforms/Linux/Base/**.h",
      "./Platforms/Linux/Base/**.inl",
      "./Platforms/Linux/Base/**.cpp",
    }
    includedirs {
      "./Platforms/Linux/",
    }

  -- OSX
  configuration( "osx-*" )
    files {
      "./Platforms/OSX/Base/**.h",
      "./Platforms/OSX/Base/**.inl",
      "./Platforms/OSX/Base/**.cpp",
      "./Platforms/OSX/Base/**.mm",
    }
    includedirs {
      "./Platforms/OSX/",
    }

  -- Android
  configuration( "android-*" )
    files {
      "./Platforms/Android/Base/**.h",
      "./Platforms/Android/Base/**.inl",
      "./Platforms/Android/Base/**.cpp",
    }
    includedirs {
      "./Platforms/Android/",
    }

  -- asmjs
  configuration( "html5-clang-asmjs" )
    files {
      "./Platforms/HTML5/Base/**.h",
      "./Platforms/HTML5/Base/**.inl",
      "./Platforms/HTML5/Base/**.cpp",
    }
    includedirs {
      "./Platforms/HTML5/",
    }
