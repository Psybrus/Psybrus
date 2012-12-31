project "External_aptk"
	kind "StaticLib"
	language "C"
	files { "./inc/aptk/**.h", "./src/aptk/**.c" }
	includedirs { "./inc" }

