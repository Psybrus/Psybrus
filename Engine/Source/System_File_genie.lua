project "Engine_System_File"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/File/**.h", 
    "./Shared/System/File/**.inl", 
    "./Shared/System/File/**.cpp", 
    "./Platforms/Windows/System/File/**.h", 
    "./Platforms/Windows/System/File/**.inl", 
    "./Platforms/Windows/System/File/**.cpp", 
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
