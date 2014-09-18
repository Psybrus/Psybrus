
local action = _ACTION or ""

-- Setup the main solution.
solution "Psybrus"
	location ( "Build/" .. action )
	configurations { "Debug", "Release", "Production" }
	configuration "vs*"
		defines { "STATICLIB", "_CRT_SECURE_NO_WARNINGS", "_STATIC_CPPLIB" }	

	configuration "Debug"
		targetdir ( "Build/" .. action .. "/bin/Debug" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "DEBUG", "PSY_DEBUG", "PSY_IMPORT_PIPELINE" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "Symbols" }

	configuration "Release"
		targetdir ( "Build/" .. action .. "/bin/Release" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_RELEASE", "PSY_IMPORT_PIPELINE" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "Symbols", "Optimize" }

	configuration "Production"
		targetdir ( "Build/" .. action .. "/bin/Production" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_PRODUCTION" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "NoFramePointer", "Optimize" }

	-- Build externals.
	dofile ("External/genie.lua")

	-- Build engine.
	dofile ("Engine/genie.lua")

	-- Build LD23Game.
--	dofile ("7DFPSGame/genie.lua")

