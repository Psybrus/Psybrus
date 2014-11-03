-- Project.lua
-- Common project creation functions.

-- Locals
local EXTERNAL_PREFIX = "External_"

-- Common project setup.
function PsyProjectCommon( _name )
	project( _name )

	-- Common flags for all configurations.
	configuration "*"
		flags { "StaticRuntime", "FloatFast", "NativeWChar" }

	-- Windows config defines + flags.
	configuration "windows"
		defines { "WINDOWS", "_WIN32", "WIN32" }

	configuration "vs*"
		flags { "EnableSSE", "EnableSSE2" }

	-- Linux config defines.
	configuration "linux"
		defines { "linux", "__linux" }

	-- Common visual studio crap.
	configuration "vs*"

	-- Thread sanitiser support for later
	configuration "gmake"
		--buildoptions { "-fsanitize=thread", "-fPIE", "-pie" }
		--linkoptions { "-fsanitize=thread", "-fPIE", "-pie", "-ltsan" }

	-- Common defines for build targets across all types of project.
	configuration "Debug"
		defines { "_DEBUG", "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Symbols", "Optimize" }

	configuration "Profile"
		defines { "NDEBUG" }
		flags { "Symbols", "Optimize" }

	configuration "Production"
		defines { "NDEBUG" }
		flags { "Symbols", "Optimize" }

	-- Terminate project.
	configuration "*"
end

-- Common engine project.
function PsyProjectCommonEngine( _name )
	PsyProjectCommon( _name )

	-- Enable C++11.
	configuration "gmake"
		buildoptions { "-std=c++11" }

	-- Extra warnings + fatal warnings.
	configuration "vs*"
		flags { "ExtraWarnings", "FatalWarnings" }

	-- Defines for all configurations
	configuration "Debug"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_DEBUG", "PSY_IMPORT_PIPELINE" }

	configuration "Release"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_RELEASE", "PSY_IMPORT_PIPELINE" }

	configuration "Profile"
		defines { "PSY_USE_PROFILER=1" }
		defines { "PSY_RELEASE", "PSY_IMPORT_PIPELINE" }

	configuration "Production"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_PRODUCTION" }

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

	PsyAddSystemLibs()

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


-- Add engine link.
function PsyAddEngineLinks( _name )
	links( _name )
end


-- Add external link.
function PsyAddExternalLinks( _name )
	links( _name )
end

