project "External_libb64"
	kind "StaticLib"
	language "C"
	files { "./src/**.h", "./src/**.c" }
	includedirs { "./include" }
