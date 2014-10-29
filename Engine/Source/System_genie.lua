PsyProjectEngineLib( "Engine_System" )
  configuration "*"

  	files {
      "./Shared/System/*.h", 
      "./Shared/System/*.inl", 
      "./Shared/System/*.cpp", 
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
      BOOST_INCLUDE_PATH,
    }

    -- GLEW config.
    defines { "GLEW_STATIC" }

    links {
      -- Engine libs.
      "Engine_Base",
      "Engine_Events",
      "Engine_Import",
      "Engine_Reflection",
      "Engine_Math",
      "Engine_Serialisation",
    }

  configuration "linux"
      files {
          "./Platforms/Linux/System/*.h", 
          "./Platforms/Linux/System/*.inl", 
          "./Platforms/Linux/System/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "windows"
        files {
            "./Platforms/Windows/System/*.h", 
            "./Platforms/Windows/System/*.inl", 
            "./Platforms/Windows/System/*.cpp", 
        }
        includedirs {
            "./Platforms/Windows/",
        }

	    libdirs {
           BOOST_LIB_PATH
        }
