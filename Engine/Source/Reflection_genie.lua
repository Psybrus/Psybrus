PsyProjectEngineLib( "Reflection" )
  configuration "*"
  	files {
      "./Shared/Reflection/**.h", 
      "./Shared/Reflection/**.inl",
      "./Shared/Reflection/**.cpp",
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
    "Engine_Base",
  }

  PsyAddExternalLinks {
    "libb64",
  }
