-- Project.lua
-- Common project creation functions.

-- Locals
local EXTERNAL_PREFIX = "External_"

-- Common project setup.
function PsyProjectCommon( _name, _lang )
	project( _name )

	-- Common flags for all configurations.
	configuration "*"
		flags { "StaticRuntime", "FloatFast", "NativeWChar" }

	-- Windows config defines + flags.
	configuration "windows-*"
		defines { "WINDOWS", "_WIN32", "WIN32" }

	configuration "vs* and x32"
		flags { "EnableSSE", "EnableSSE2" }

	-- Linux config defines.
	configuration "linux-*"
		defines { "linux", "__linux" }

	-- Common visual studio crap.
	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }

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
		flags { "Symbols" }

	configuration "Profile"
		defines { "NDEBUG" }
		flags { "Symbols" }

	configuration "Production"
		defines { "NDEBUG" }
		flags { "Symbols" }

	-- Optimised builds.
	configuration { "windows-* or linux-*", "Release" }
		flags { "Optimize" }

	configuration { "windows-* or linux-*", "Profile" }
		flags { "Optimize" }

	configuration { "windows-* or linux-*", "Production" }
		flags { "Optimize" }

	-- Setup language specific support.
	languageOptions = {
		[ "C" ] = "C",
		[ "C++" ] = "C++",
		[ "C++11" ] = "C++",
		[ "C++14" ] = "C++",
		[ "C++1z" ] = "C++",
		[ "C++17" ] = "C++"
	}

	gccLanguageOptions = {
		[ "C" ] = {},
		[ "C++" ] = {},
		[ "C++11" ] = { "-std=c++11" },
		[ "C++14" ] = { "-std=c++14" },
		[ "C++1z" ] = { "-std=c++1z" },
		[ "C++17" ] = { "-std=c++17" }
	}

	clangLanguageOptions = {
		[ "C" ] = {},
		[ "C++" ] = { "-stdlib=libc++" },
		[ "C++11" ] = { "-stdlib=libc++", "-std=c++11" },
		[ "C++14" ] = { "-stdlib=libc++", "-std=c++14" },
		[ "C++1z" ] = { "-stdlib=libc++", "-std=c++1z" },
		[ "C++17" ] = { "-stdlib=libc++", "-std=c++17" }
	}

	configuration "*"
		language( languageOptions[ _lang ] )

	configuration "*-gcc"
		buildoptions( gccLanguageOptions[ _lang ] )

	configuration "*-clang or asmjs"
		buildoptions( clangLanguageOptions[ _lang ] )

	-- Terminate project.
	configuration "*"
end

-- Common engine project.
function PsyProjectCommonEngine( _name )
	PsyProjectCommon( _name, "C++11" )

	-- Enable C++11.
	configuration "gmake"
		buildoptions { "-std=c++11" }
		buildoptions { "-stdlib=libc++" }
		links {
			"c++"
		}

	-- Extra warnings + fatal warnings.
	configuration "vs*"
		flags { "ExtraWarnings" }

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

	-- Add default include paths.
	configuration( "*" )
		includedirs {
			"../../External/imgui",
		}

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
	group( _name )
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
	group( _name )
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
	configuration { "asmjs", "Debug" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) cp -r ../../Dist/PackedContent ./",
			"$(SILENT) echo Running asmjs finalise \\(Debug\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O0 --memory-init-file 1 --js-opts 0 -g3 -s ASM_JS=1 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=268435456 \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	configuration { "asmjs", "Release" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) cp -r ../../Dist/PackedContent ./",
			"$(SILENT) echo Running asmjs finalise \\(Release\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O3 --memory-init-file 1 --js-opts 1 -g3 -s ASM_JS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=268435456 \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	configuration { "asmjs", "Production" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) cp -r ../../Dist/PackedContent ./",
			"$(SILENT) echo Running asmjs finalise \\(Production\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O3 --memory-init-file 1 --js-opts 1 --llvm-lto 1 -s ASM_JS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=268435456 \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	-- Terminate project.
	configuration "*"
end


-- Setup engine lib project.
function PsyProjectEngineLib( _name )
	group( "Psybrus Engine" )

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
function PsyProjectExternalLib( _name, _lang )
	group( "Psybrus External" )

	-- Prepend "External_"
	_name = "External_" .. _name

	-- Setup common project stuff.
	PsyProjectCommon( _name, _lang )
	print( "Adding External Library: " .. _name )

	configuration "*"
		kind "StaticLib"
		flags { "Optimize" }

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

