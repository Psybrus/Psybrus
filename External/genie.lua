local action = _ACTION or ""

newoption {
	trigger     = "external-solution",
	description = "Generate external solution",
}

-- Default library prefixes.
EXTERNAL_PROJECT_PREFIX = "External_"
EXTERNAL_PROJECT_KIND = "StaticLib"

-- Setup the external solution.
if( _OPTIONS["external-solution"] ) then
	solution "External"
		location ( "Build/" .. action )
		configurations { "Debug", "Release", "Production" }
		configuration "vs*"
			defines { "_CRT_SECURE_NO_WARNINGS" }	

		configuration "Debug"
			targetdir ( "Build/" .. action .. "/bin/Debug" )
			defines { "WINDOWS", "_WIN32", "WIN32", "DEBUG" }
			flags { "NativeWChar", "Symbols" }

		configuration "Release"
			targetdir ( "Build/" .. action .. "/bin/Release" )
			defines { "WINDOWS", "_WIN32", "WIN32", "NDEBUG" }
			flags { "NativeWChar", "Optimize" }

	-- Prefix libraries
	EXTERNAL_PROJECT_PREFIX = ""
	EXTERNAL_PROJECT_KIND = "StaticLib"
end

-- In repo.
dofile ("bullet_genie.lua")
dofile ("freetype_genie.lua")
dofile ("glew_genie.lua")
dofile ("jsoncpp_genie.lua")
dofile ("libb64_genie.lua")
dofile ("pcre_genie.lua")
dofile ("png_genie.lua")
dofile ("rapidxml_genie.lua")
dofile ("squish_genie.lua")
dofile ("webby_genie.lua")
dofile ("zlib_genie.lua")

-- Submodules.
dofile ("assimp_genie.lua")
dofile ("enet_genie.lua")
dofile ("HLSLCrossCompiler_genie.lua")
dofile ("SoLoud_genie.lua")
