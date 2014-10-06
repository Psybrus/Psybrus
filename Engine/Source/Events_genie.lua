project "Engine_Events"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Events/**.h", 
    "./Shared/Events/**.inl", 
    "./Shared/Events/**.cpp", 
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
   		}

   	configuration "vs2012"
   		links {
   		}
