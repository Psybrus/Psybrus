PsyProjectExternalLib( "HLSLCrossCompiler" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	configuration "windows or linux-gcc or linux-clang"
		files { 
			"./HLSLCrossCompiler/include/**.h", 
			"./HLSLCrossCompiler/src/**.h", 
			"./HLSLCrossCompiler/src/**.c",
		}

		includedirs { 
			"./HLSLCrossCompiler/include", 
			"./HLSLCrossCompiler/src", 
			"./HLSLCrossCompiler/src/cbstring" ,
		}
