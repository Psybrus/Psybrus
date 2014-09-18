project "External_enet"
	kind "StaticLib"
	language "C"
	files { "./enet/**.h", "./enet/**.c" }
	excludes { "./enet/unix.c" }
	includedirs { "./enet/include" }
