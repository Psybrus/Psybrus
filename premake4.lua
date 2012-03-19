
local action = _ACTION or ""

-- Setup the main solution.
solution "Psybrus"
	location ( "Build/" .. action )
	configurations { "Debug", "Release", "Production" }

	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS", "_STATIC_CPPLIB", "_HAS_EXCEPTIONS=0" }	

	configuration "Debug"
		targetdir ( "Build/" .. action .. "/bin/Debug" )
		defines { "WINDOWS", "_WIN32", "WIN32", "DEBUG", "PSY_DEBUG", "PSY_SERVER" }
		flags { "StaticRuntime", "EnableSSE", "EnableSSE2", "FloatFast", "NativeWChar", "NoPCH", "NoRTTI", "NoExceptions", "Symbols" }

	configuration "Release"
		targetdir ( "Build/" .. action .. "/bin/Release" )
		defines { "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_RELEASE", "PSY_SERVER" }
		flags { "StaticRuntime", "EnableSSE", "EnableSSE2", "FloatFast", "NativeWChar", "NoPCH", "NoRTTI", "NoExceptions", "Symbols", "Optimize" }

	configuration "Production"
		targetdir ( "Build/" .. action .. "/bin/Production" )
		defines { "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_PRODUCTION" }
		flags { "StaticRuntime", "EnableSSE", "EnableSSE2", "FloatFast", "NativeWChar", "NoPCH", "NoRTTI", "NoExceptions", "NoFramePointer", "Optimize" }

	-- Build externals.
	dofile ("External/premake4.lua")

	-- Build engine.
	dofile ("Engine/premake4.lua")

	-- Build example game.
	dofile ("ExampleGame/premake4.lua")

	-- Build template game.
	dofile ("TemplateGame/premake4.lua")
