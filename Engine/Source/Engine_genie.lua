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
            "../../External/freetype/include",
            "../../External/glew/include",
            "../../External/jsoncpp/include/",
            "../../External/libb64/include/",
            "../../External/png/",
            "../../External/squish/",
            "../../External/webby/",
            "../../External/zlib/",
            BOOST_INCLUDE_PATH,  
        }

        libdirs {
           BOOST_LIB_PATH
        }

    configuration "linux-*"
        files {
            "./Platforms/Linux/*.h", 
            "./Platforms/Linux/*.inl", 
            "./Platforms/Linux/*.cpp", 
        }
        includedirs {
            "./Platforms/Linux/",
        }

    configuration "osx-*"
        files {
            "./Platforms/OSX/*.h", 
            "./Platforms/OSX/*.inl", 
            "./Platforms/OSX/*.cpp", 
            "./Platforms/OSX/*.mm", 
        }
        includedirs {
            "./Platforms/OSX/",
            "/usr/local/Cellar/sdl2/2.0.3/include" 
        }

    configuration "android-*"
        files {
            "./Platforms/Android/*.h", 
            "./Platforms/Android/*.inl", 
            "./Platforms/Android/*.cpp"
        }
        includedirs {
            "./Platforms/Android/",
        }

    configuration "html5-clang-asmjs"
        files {
            "./Platforms/HTML5/*.h", 
            "./Platforms/HTML5/*.inl", 
            "./Platforms/HTML5/*.cpp", 
        }
        includedirs {
            "./Platforms/HTML5/",
        }

    configuration( "windows-* or winphone-*" )
        files {
            "./Platforms/Windows/*.h", 
            "./Platforms/Windows/*.inl", 
            "./Platforms/Windows/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }
