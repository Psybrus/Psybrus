PsyProjectEngineLib( "Engine_System_Network" )
  configuration "*"
  	files {
      "./Shared/System/Network/**.h", 
      "./Shared/System/Network/**.inl", 
      "./Shared/System/Network/**.cpp", 
      "./Platforms/Windows/System/Network/**.h", 
      "./Platforms/Windows/System/Network/**.inl", 
      "./Platforms/Windows/System/Network/**.cpp", 
    }
  	includedirs {
      "./Shared/",
      "./Platforms/Windows/",
      "../../External/enet/include",
      "../../External/jsoncpp/include/",
      "../../External/libb64/include/",
      "../../External/webby/",
      BOOST_INCLUDE_PATH,
    }

	configuration "windows"
	    libdirs {
           BOOST_LIB_PATH
        }

   		links {
        -- Engine libs.
        "Engine_System",

   			-- External libs.
        "External_enet",
        "External_webby",
   		}

   	configuration "vs2012"
   		links {
   		}
