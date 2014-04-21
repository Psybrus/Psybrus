project "Engine_Math"
	kind "StaticLib"
	language "C++"
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
    psybrusSDK .. "/External/pcre/",
    psybrusSDK .. "/External/zlib/",
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
