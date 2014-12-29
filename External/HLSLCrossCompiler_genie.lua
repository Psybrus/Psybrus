if PsyProjectExternalLib( "HLSLCrossCompiler", "C" ) then
	kind ( EXTERNAL_PROJECT_KIND )
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
end
