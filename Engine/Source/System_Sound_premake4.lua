project "Engine_System_Sound"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Sound/**.h", 
    "./Shared/System/Sound/**.inl", 
    "./Shared/System/Sound/**.cpp", 
    "./Platforms/Windows/System/Sound/**.h", 
    "./Platforms/Windows/System/Sound/**.inl", 
    "./Platforms/Windows/System/Sound/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
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
        "External_jsoncpp",
        "External_libb64"
   		}

   	configuration "vs2012"
   		links {
   		}
