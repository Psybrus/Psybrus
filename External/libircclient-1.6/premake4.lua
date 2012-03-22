project "External_libircclient-1.6"
	kind "StaticLib"
	language "C"
	files { "./include/**.h" }
	includedirs { "./include" }
