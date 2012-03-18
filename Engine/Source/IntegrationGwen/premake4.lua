project "EngineIntegrationGwen"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp" }
	includedirs { "./**", "../Shared/**" }
