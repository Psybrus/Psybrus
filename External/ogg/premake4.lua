project "External_ogg"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
	includedirs { ".", "./include" }
