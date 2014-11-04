local action = _ACTION or ""

newoption {
	trigger     = "external-solution",
	description = "Generate external solution",
}

-- Default library prefixes.
EXTERNAL_PROJECT_PREFIX = "External_"
EXTERNAL_PROJECT_KIND = "StaticLib"

-- In repo.
dofile ("bullet_genie.lua")
dofile ("freetype_genie.lua")
dofile ("jsoncpp_genie.lua")
dofile ("libb64_genie.lua")
dofile ("pcre_genie.lua")
dofile ("png_genie.lua")
dofile ("squish_genie.lua")
dofile ("zlib_genie.lua")

if _OPTIONS[ "toolchain" ] ~= "asmjs" then
	dofile ("glew_genie.lua")
	dofile ("webby_genie.lua")
end

-- Submodules.
dofile ("assimp_genie.lua")
dofile ("enet_genie.lua")
dofile ("HLSLCrossCompiler_genie.lua")
dofile ("SoLoud_genie.lua")
