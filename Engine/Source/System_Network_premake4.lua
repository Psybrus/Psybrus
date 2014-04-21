project "Engine_System_Network"
	kind "StaticLib"
	language "C++"
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
    psybrusSDK .. "/External/enet/include",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    psybrusSDK .. "/External/mongoose/",
    boostInclude,
  }

	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_System",

   			-- External libs.
        "External_enet",
        "External_jsoncpp",
        "External_libb64",
        "External_mongoose",
   		}

   	configuration "vs2012"
   		links {
   		}
