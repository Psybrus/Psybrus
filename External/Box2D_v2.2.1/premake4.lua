project "External_Box2D_v2.2.1"
	kind "StaticLib"
	language "C++"
	files { "./Box2D/**.h", "./Box2D/**.cpp" }
	includedirs { "./" }
