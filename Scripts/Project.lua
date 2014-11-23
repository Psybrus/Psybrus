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
	configuration "windows-*"
		defines { "WINDOWS", "_WIN32", "WIN32" }

	configuration "vs*"
		flags { "EnableSSE", "EnableSSE2" }

	-- Linux config defines.
	configuration "linux-*"
		defines { "linux", "__linux" }

	-- Common visual studio crap.
	configuration "vs*"

	-- Thread sanitiser support for later
	configuration "gmake"
		-- buildoptions { "-fsanitize=thread", "-fPIE", "-pie" }
		-- linkoptions { "-fsanitize=thread", "-fPIE", "-pie", "-ltsan" }

	-- Setup dynamic linking for backtrace support.
	configuration { "Debug", "gmake" }
		linkoptions { "-rdynamic" }

	configuration { "Release", "gmake" }
		linkoptions { "-rdynamic" }

	configuration { "Profile", "gmake" }
		linkoptions { "-rdynamic" }

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
		defines { "PSY_DEBUG" }

	configuration "Release"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_RELEASE" }

	configuration "Profile"
		defines { "PSY_USE_PROFILER=1" }
		defines { "PSY_RELEASE" }

	configuration "Production"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_PRODUCTION" }

	-- Import pipeline.
	configuration "windows-* or linux-*"
		defines { "PSY_IMPORT_PIPELINE" }


	-- Include paths.
	configuration( "windows-*" )
		includedirs {
			"./Platforms/Windows/",
		}

	configuration( "linux-*" )
		includedirs {
			"./Platforms/Linux/",
		}

	configuration( "asmjs" )
		includedirs {
			"./Platforms/HTML5/",
			"$(EMSCRIPTEN)/system/lib/libcxxabi/include",
		}

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

	configuration "*"
		local targetNamePrefix = _name .. "-" .. _ACTION .. "-" .. _OPTIONS[ "toolchain" ]
		targetname( targetNamePrefix .. "-" )
	
	--
	configuration "windows-* or linux-*"
		targetdir ( "../Dist" )

	PsyAddSystemLibs()

	-- asmjs post build.
	configuration { "asmjs" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) cp -r ../../Dist/PackedContent ./",
			"$(SILENT) echo Running asmjs finalise.",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			--"$(SILENT) $(EMSCRIPTEN)/emcc -v -O2 --js-opts 0 -g4 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=268435456 \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O0 --js-opts 0 -g4 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=268435456 \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	-- Terminate project.
	configuration "*"
end


-- Setup engine lib project.
function PsyProjectEngineLib( _name )
	-- Prepend "Engine_"
	_name = "Engine_" .. _name

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
	-- Prepend "External_"
	_name = "External_" .. _name

	-- Setup common project stuff.
	PsyProjectCommon( _name )
	print( "Adding External Library: " .. _name )

	configuration "*"
		kind "StaticLib"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	-- Terminate project.
	configuration "*"

	return true;
end


-- Add engine link.
function PsyAddEngineLinks( _names )
	for i, name in ipairs( _names ) do
		links { "Engine_" .. name }
	end
end

-- Add external link.
function PsyAddExternalLinks( _names )
	for i, name in ipairs( _names ) do
		links { "External_" .. name }
	end
end

