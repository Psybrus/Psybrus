PsyProjectEngineLib( "Math" )
  configuration "*"
  	files {
      "./Shared/Math/**.h", 
      "./Shared/Math/**.inl", 
      "./Shared/Math/**.cpp",
      "./Platforms/Windows/Math/**.h",
      "./Platforms/Windows/Math/**.inl",
      "./Platforms/Windows/Math/**.cpp",
    }

  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/zlib/",
      BOOST_INCLUDE_PATH,
    }

	configuration "windows-*"
	    libdirs {
           BOOST_LIB_PATH
        }

   		PsyAddEngineLinks {
   			"Base",
        "Reflection",
   		}
