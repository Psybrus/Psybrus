project "External_glew"
	kind "StaticLib"
	language "C++"
	files { "./include/**.h", "./src/glew.c" }
	includedirs { "./include" }
	defines { "GLEW_STATIC" }