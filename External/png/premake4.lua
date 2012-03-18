project "External_png"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
	includedirs { ".", "../zlib" }
