project "External_squish"
	kind "StaticLib"
	language "C++"
	files { "./*.h", "./*.cpp" }
	includedirs { "." }
	defines{ "SQUISH_USE_SSE=1" }
