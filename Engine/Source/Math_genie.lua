PsyProjectEngineLib( "Engine_Math" )
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
      "../../External/pcre/",
      "../../External/zlib/",
      boostInclude,
    }

	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
   			-- External libs.
   			"Engine_Base",
        "Engine_Reflection",
   		}

   	configuration "vs2012"
   		links {
   		}
