
local action = _ACTION or ""

-- Setup the main solution.
solution "Psybrus"
	location ( "Build/" .. action )
	configurations { "Debug", "Release", "Production" }

	configuration { "native" }
		openal32LibPath = psybrusSDK .. "/External/openal/libs/Win32/"

	configuration { "x32" }
		openal32LibPath = psybrusSDK .. "/External/openal/libs/Win32/"

--	configuration { "x64" }
--		openal32LibPath = psybrusSDK .. "/External/openal/libs/Win64/"

	configuration "vs*"
		defines { "STATICLIB", "_CRT_SECURE_NO_WARNINGS", "_STATIC_CPPLIB", "_HAS_EXCEPTIONS=0" }	

	configuration "Debug"
		targetdir ( "Build/" .. action .. "/bin/Debug" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "DEBUG", "PSY_DEBUG", "PSY_SERVER" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "NoPCH", "Symbols" }

	configuration "Release"
		targetdir ( "Build/" .. action .. "/bin/Release" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_RELEASE", "PSY_SERVER" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "NoPCH", "Symbols", "Optimize" }

	configuration "Production"
		targetdir ( "Build/" .. action .. "/bin/Production" )
		defines { "STATICLIB", "WINDOWS", "_WIN32", "WIN32", "NDEBUG", "PSY_PRODUCTION" }
		flags { "StaticRuntime", "FloatFast", "NativeWChar", "NoPCH", "NoFramePointer", "Optimize" }

	-- Build externals.
	dofile ("External/premake4.lua")

	-- Build engine.
	dofile ("Engine/premake4.lua")

	-- Build LD23Game.
--	dofile ("7DFPSGame/premake4.lua")

