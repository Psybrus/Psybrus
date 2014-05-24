project "External_HLSLCrossCompiler"
	kind "StaticLib"
	language "C"
	files { "./include/**.h", "./src/**.h", "./src/**.c" }
	includedirs { "./include", "./src/cbstring" }
