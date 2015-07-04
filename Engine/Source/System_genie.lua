PsyProjectEngineLib( "System" )
  configuration "*"

  	files {
      "./Shared/System/*.h", 
      "./Shared/System/*.inl", 
      "./Shared/System/*.cpp", 
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

    -- GLEW config.
    defines { "GLEW_STATIC" }

    PsyAddEngineLinks {
      "Base",
      "Events",
      "Import",
      "Reflection",
      "Math",
      "Serialisation",
    }

  configuration "linux-*"
      files {
          "./Platforms/Linux/System/*.h", 
          "./Platforms/Linux/System/*.inl", 
          "./Platforms/Linux/System/*.cpp", 
      }
      includedirs {
          "./Platforms/Linux/",
      }

  configuration "osx-*"
      files {
          "./Platforms/OSX/System/*.h", 
          "./Platforms/OSX/System/*.inl", 
          "./Platforms/OSX/System/*.cpp", 
          "./Platforms/OSX/System/*.mm", 
      }
      includedirs {
          "./Platforms/OSX/",
      }

  configuration "windows-*"
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
