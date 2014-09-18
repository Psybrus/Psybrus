project "External_glew"
	kind "StaticLib"
	language "C++"
	files { "./glew/include/**.h", "./glew/src/glew.c" }
	includedirs { "./glew/include" }
	defines { "GLEW_STATIC" }