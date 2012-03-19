project "External_gwen"
	kind "StaticLib"
	language "C++"
	files { "./include/**.h", "./src/**.h", "./src/**.cpp", "./UnitTest/**.cpp" }
	includedirs { "./include" }
