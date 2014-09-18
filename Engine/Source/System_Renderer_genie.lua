project "Engine_System_Renderer"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Renderer/**.h", 
    "./Shared/System/Renderer/**.inl", 
    "./Shared/System/Renderer/**.cpp", 
    "./Platforms/Windows/System/Renderer/**.h", 
    "./Platforms/Windows/System/Renderer/**.inl", 
    "./Platforms/Windows/System/Renderer/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/glew/include",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    boostInclude,
  }

  -- GLEW config.
  defines { "GLEW_STATIC" }


	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_System",

   			-- External libs.
        "External_glew",
        "External_jsoncpp",
        "External_libb64",
   		}

   	configuration "vs2012"
   		links {
   		}
