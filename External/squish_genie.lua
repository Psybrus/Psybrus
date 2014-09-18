project "External_squish"
	kind "StaticLib"
	language "C++"
	files { "./squish/*.h", ".squish/*.cpp" }
	includedirs { ".squish/" }
	defines{ "SQUISH_USE_SSE=1" }
