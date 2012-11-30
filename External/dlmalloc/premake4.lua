project "External_dlmalloc"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.cpp" }
	includedirs { "./" }
