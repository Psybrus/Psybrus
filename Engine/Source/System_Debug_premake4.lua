project "Engine_System_Debug"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Debug/**.h", 
    "./Shared/System/Debug/**.inl", 
    "./Shared/System/Debug/**.cpp", 
    "./Platforms/Windows/System/Debug/**.h", 
    "./Platforms/Windows/System/Debug/**.inl", 
    "./Platforms/Windows/System/Debug/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
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
        "Engine_System_Content",

   			-- External libs.
        "External_jsoncpp",
        "External_libb64",
        "External_mongoose",
   		}

   	configuration "vs2012"
   		links {
   		}
