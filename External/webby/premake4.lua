project "External_webby"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
	includedirs { "." }
