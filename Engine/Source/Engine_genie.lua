PsyProjectEngineLib( "Engine" )
    configuration "*"
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
            "../../External/SDL2/include/",
            "../../External/squish/",
            "../../External/webby/",
            "../../External/zlib/",
            boostInclude,  
        }

        libdirs {
           boostLib
        }

        links {
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
