project "External_RakNet"
	kind "StaticLib"
	language "C++"
	files { "./Source/*.h", "./Source/*.hpp", "./Source/*.cpp" }
	includedirs { "./Source" }
