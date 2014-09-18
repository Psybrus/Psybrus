project "External_libb64"
	kind "StaticLib"
	language "C"
	files { "./libb64/src/**.h", "./libb64/src/**.c" }
	includedirs { "./libb64/include" }
