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
        "../../External/bullet/src",
        "../../External/enet/include",
        "../../External/freetype/include",
        "../../External/glew/include",
        "../../External/pcre/",
        "../../External/jsoncpp/include/",
        "../../External/libb64/include/",
        "../../External/png/",
        "../../External/squish/",
        "../../External/webby/",
        "../../External/zlib/",
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
