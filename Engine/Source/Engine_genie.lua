project "Engine"
    kind "StaticLib"
    language "C++"   
    files {
        "./Shared/*.h", 
        "./Shared/*.inl", 
        "./Shared/*.cpp", 
    }
 
    includedirs {
        "./Shared/",
        psybrusSDK .. "/External/bullet/src",
        psybrusSDK .. "/External/enet/include",
        psybrusSDK .. "/External/freetype/include",
        psybrusSDK .. "/External/glew/include",
        psybrusSDK .. "/External/pcre/",
        psybrusSDK .. "/External/jsoncpp/include/",
        psybrusSDK .. "/External/libb64/include/",
        psybrusSDK .. "/External/png/",
        psybrusSDK .. "/External/squish/",
        psybrusSDK .. "/External/webby/",
        psybrusSDK .. "/External/zlib/",
        boostInclude,  
    }

    libdirs {
       boostLib
    }

    links {
    -- Engine libs.
    "Engine_Base",
    "Engine_Events",
    "Engine_Import",
    "Engine_Math",
    "Engine_Reflection",
    "Engine_Serialisation",
    "Engine_System",
    "Engine_System_Content",
    "Engine_System_Debug",
    "Engine_System_File",
    "Engine_System_Network",
    "Engine_System_Os",
    "Engine_System_Renderer",
    "Engine_System_Scene",
    "Engine_System_Sound",
    }

    configuration "linux"
        files {
            "./Platforms/Linux/*.h", 
            "./Platforms/Linux/*.inl", 
            "./Platforms/Linux/*.cpp", 
        }
        includedirs {
            "./Platforms/Linux/",
        }

	configuration "windows"
        files {
            "./Platforms/Windows/*.h", 
            "./Platforms/Windows/*.inl", 
            "./Platforms/Windows/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

   	configuration "vs2012"
   		links {
   		}
