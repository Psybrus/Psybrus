project "External_enet"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
	excludes { "unix.c" }
	includedirs { "./include" }
