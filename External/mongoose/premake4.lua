project "External_mongoose"
	kind "StaticLib"
	language "C"
	files { "./mongoose.h", "./mongoose.c" }
	includedirs { "./include" }
	defines { "GLEW_STATIC" }