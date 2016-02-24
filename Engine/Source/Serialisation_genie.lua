PsyProjectEngineLib( "Serialisation" )
  configuration "*"
  	files {
      "./Shared/Serialisation/**.h", 
      "./Shared/Serialisation/**.inl",
      "./Shared/Serialisation/**.cpp",
    }
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/zlib/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/"
    }

	configuration "windows-*"

  PsyAddEngineLinks {
    "Base",
    "Reflection",
  }

  PsyAddEngineLinks {
  	"zlib",
    "jsoncpp",
    "libb64",
  }
