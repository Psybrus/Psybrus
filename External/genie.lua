local action = _ACTION or ""

newoption {
	trigger     = "external-solution",
	description = "Generate external solution",
}

-- Default library prefixes.
EXTERNAL_PROJECT_PREFIX = "External_"
EXTERNAL_PROJECT_KIND = "StaticLib"

dofile ("bullet_genie.lua")
dofile ("freetype_genie.lua")
dofile ("jsoncpp_genie.lua")
dofile ("libb64_genie.lua")
dofile ("png_genie.lua")
dofile ("zlib_genie.lua")
dofile ("SoLoud_genie.lua")

-- Stuff we don't want to build for asmjs (import pipeline or other stuff only)
if _OPTIONS[ "toolchain" ] ~= "asmjs" then
	dofile ("rapidxml_genie.lua")
	dofile ("glew_genie.lua")
	dofile ("webby_genie.lua")
	dofile ("squish_genie.lua")
	dofile ("assimp_genie.lua")
	dofile ("glsl-optimizer_genie.lua")
	dofile ("HLSLCrossCompiler_genie.lua")
	dofile ("hlsl2glslfork_genie.lua")
	dofile ("ThinkGear_genie.lua")
end
