project "External_webby"
	kind "StaticLib"
	language "C"
	files { "./webby/**.h", "./webby/**.c" }
	includedirs { "./webby/" }
