project "External_gwen"
	kind "StaticLib"
	language "C++"
	files { "./src/**.h", "./src/**.cpp", "./UnitTest/**.cpp" }
	includedirs { "./include" }
