project "Engine_System_Os"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Os/**.h", 
    "./Shared/System/Os/**.inl", 
    "./Shared/System/Os/**.cpp", 
    "./Platforms/Windows/System/Os/**.h", 
    "./Platforms/Windows/System/Os/**.inl", 
    "./Platforms/Windows/System/Os/**.cpp", 
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
   		}

   	configuration "vs2012"
   		links {
   		}
