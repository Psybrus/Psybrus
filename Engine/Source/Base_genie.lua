PsyProjectEngineLib( "Engine_Base" )
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

    links {
      -- External libs.
      "External_pcre",
      "External_zlib",
    }

  -- Windows
  configuration( "windows" )
    files {
      "./Platforms/Windows/Base/**.h",
      "./Platforms/Windows/Base/**.inl",
      "./Platforms/Windows/Base/**.cpp",
    }
    includedirs {
      "./Platforms/Windows/",
    }

  -- Linux
  configuration( "linux" )
    files {
      "./Platforms/Linux/Base/**.h",
      "./Platforms/Linux/Base/**.inl",
      "./Platforms/Linux/Base/**.cpp",
    }
    includedirs {
      "./Platforms/Linux/",
    }
