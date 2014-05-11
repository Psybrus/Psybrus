project "Engine_Base"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/Base/**.h", 
    "./Shared/Base/**.inl", 
    "./Shared/Base/**.cpp",
    "./Platforms/Windows/Base/**.h",
    "./Platforms/Windows/Base/**.inl",
    "./Platforms/Windows/Base/**.cpp",
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
   			"External_pcre",
   			"External_zlib",
   		}

   	configuration "vs2012"
   		links {
   		}
