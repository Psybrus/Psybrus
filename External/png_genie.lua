project "External_png"
	kind "StaticLib"
	language "C"
	files { "./png/**.h", "./png/**.c" }
	includedirs { ".png/", "./zlib/" }
