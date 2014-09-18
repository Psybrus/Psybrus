project "External_HLSLCrossCompiler"
	kind "StaticLib"
	language "C"
	files { 
		"./HLSLCrossCompiler/include/**.h", 
		"./HLSLCrossCompiler/src/**.h", 
		"./HLSLCrossCompiler/src/**.c" }
	includedirs { 
		"./HLSLCrossCompiler/include", 
		"./HLSLCrossCompiler/src/cbstring" }
