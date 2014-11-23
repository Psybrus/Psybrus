PsyProjectEngineLib( "Base" )
  configuration "*"
  	files {
      "./Shared/Base/**.h", 
      "./Shared/Base/**.inl", 
      "./Shared/Base/**.cpp",
    }

    includedirs {
      "./Shared/",
      "../../External/pcre/",
      "../../External/zlib/",
      BOOST_INCLUDE_PATH,
    }

    libdirs {
      BOOST_LIB_PATH
    }

    PsyAddExternalLinks {
      "pcre",
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

  -- asmjs
  configuration( "asmjs" )
    files {
      "./Platforms/HTML5/Base/**.h",
      "./Platforms/HTML5/Base/**.inl",
      "./Platforms/HTML5/Base/**.cpp",
    }
    includedirs {
      "./Platforms/HTML5/",
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
