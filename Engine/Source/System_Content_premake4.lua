project "Engine_System_Content"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Content/**.h", 
    "./Shared/System/Content/**.inl", 
    "./Shared/System/Content/**.cpp", 
    "./Platforms/Windows/System/Content/**.h", 
    "./Platforms/Windows/System/Content/**.inl", 
    "./Platforms/Windows/System/Content/**.cpp", 
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
        "External_libb64",
   		}

   	configuration "vs2012"
   		links {
   		}
