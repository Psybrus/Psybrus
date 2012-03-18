
local action = _ACTION or ""

-- Setup the main solution.
solution "Psybrus"
	location ( "Build/" .. action )
	configurations { "Debug", "Release", "Production" }

	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }	

	configuration "Debug"
		targetdir ( "Build/" .. action .. "/bin/Debug" )
		defines { "WINDOWS", "WIN32", "DEBUG", "PSY_DEBUG", "PSY_SERVER" }
		flags { "Symbols" }

	configuration "Release"
		targetdir ( "Build/" .. action .. "/bin/Release" )
		defines { "WINDOWS", "WIN32", "NDEBUG", "PSY_RELEASE", "PSY_SERVER" }
		flags { "Symbols", "Optimize" }

	configuration "Production"
		targetdir ( "Build/" .. action .. "/bin/Production" )
		defines { "WINDOWS", "WIN32", "NDEBUG", "PSY_PRODUCTION" }
		flags { "Optimize" }

	-- Build externals.
	dofile ("External/premake4.lua")

	-- Build engine.
	dofile ("Engine/premake4.lua")

	-- Build example game.
	dofile ("ExampleGame/premake4.lua")
