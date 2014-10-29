PsyProjectEngineLib( "Engine_Reflection" )
  configuration "*"
  	files {
      "./Shared/Reflection/**.h", 
      "./Shared/Reflection/**.inl",
      "./Shared/Reflection/**.cpp",
    }
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/pcre/",
      "../../External/zlib/",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      boostInclude,
    }

	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_Base",

   			-- External libs.
        "External_libb64",
   		}

   	configuration "vs2012"
   		links {
   		}
