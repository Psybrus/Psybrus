
-- Common project setup.
function PsyProjectCommon( _name )
	project( _name )

	-- Windows config defines.
	configuration "windows"
		defines { "WINDOWS", "_WIN32", "WIN32" }

	-- Linux config defines.
	configuration "linux"
		defines { "linux", "__linux" }

	-- Common visual studio crap.
	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS", "_STATIC_CPPLIB" }	

	-- Thread sanitiser support for later
	configuration "gmake"
		--buildoptions { "-fsanitize=thread", "-fPIE", "-pie" }
		--linkoptions { "-fsanitize=thread", "-fPIE", "-pie", "-ltsan" }

	-- Common defines for build targets across all types of project.
	configuration "Debug"
		defines { "DEBUG" }

	configuration "Release"
		defines { "NDEBUG" }

	configuration "Profile"
		defines { "NDEBUG" }

	configuration "Production"
		defines { "NDEBUG" }

	-- Terminate project.
	configuration "*"
end

-- Common engine project.
function PsyProjectCommonEngine( _name )
	PsyProjectCommon( _name )

	-- Enable C++11.
	configuration "gmake"
		buildoptions { "-std=c++11" }

	-- Common flags for all configurations.
	configuration "*"
		flags { "StaticRuntime", "EnableSSE", "EnableSSE2", "FloatFast", "NativeWChar" }

	-- Debug configuration.
	configuration "Debug"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_DEBUG", "PSY_IMPORT_PIPELINE" }
		flags { "Symbols", "Optimize" }

	configuration "Release"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_RELEASE", "PSY_IMPORT_PIPELINE" }
		flags { "Symbols", "Optimize" }

	configuration "Profile"
		defines { "PSY_USE_PROFILER=1" }
		defines { "PSY_RELEASE", "PSY_IMPORT_PIPELINE" }
		flags { "Symbols", "Optimize" }

	configuration "Production"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_PRODUCTION" }
		flags { "NoFramePointer", "Optimize" }

	-- Terminate project.
	configuration "*"
end

-- Setup a game lib project.
function PsyProjectGameLib( _name )
	PsyProjectCommonEngine( _name )
	print( "Adding Game Library: " .. _name )

	configuration "*"
		kind "StaticLib"
		language "C++"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	-- Terminate project.
	configuration "*"
end


-- Setup game exe project.
function PsyProjectGameExe( _name )
	PsyProjectCommonEngine( _name )
	print( "Adding Game Executable: " .. _name )

	configuration "*"
		kind "WindowedApp"
		language "C++"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	-- Target directories and names.
	configuration "*"
		targetdir ( "../Dist" ) -- relative to source genie.lua dir for project...?

	configuration "Debug"
		targetname( _name .. "Debug" )

	configuration "Release"
		targetname( _name .. "Release" )

	configuration "Profile"
		targetname( _name .. "Profile" )

	configuration "Production"
		targetname( _name .. "Production" )

	-- Terminate project.
	configuration "*"
end


-- Setup engine lib project.
function PsyProjectEngineLib( _name )
	PsyProjectCommonEngine( _name )
	print( "Adding Engine Library: " .. _name )

	configuration "*"
		kind "StaticLib"
		language "C++"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	-- Terminate project.
	configuration "*"
end


-- Setup external lib project.
function PsyProjectExternalLib( _name )
	PsyProjectCommon( _name )
	print( "Adding External Library: " .. _name )

	configuration "*"
		kind "StaticLib"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	-- Terminate project.
	configuration "*"
end
