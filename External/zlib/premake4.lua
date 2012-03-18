project "External_zlib"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
	includedirs { "." }
