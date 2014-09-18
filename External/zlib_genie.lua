project "External_zlib"
	kind "StaticLib"
	language "C"
	files { "./zlib/**.h", "./zlib/**.c" }
	includedirs { "./zlib/" }
