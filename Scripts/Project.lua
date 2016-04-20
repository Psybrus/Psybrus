-- Project.lua
-- Common project creation functions.

-- Locals
local EXTERNAL_PREFIX = "External_"

function PsyPlatformIncludes()
	configuration "*"

    includedirs { 
       "./", 
       "../Psybrus/Engine/Source/Shared/", 
    }

	-- External includes.
	includedirs { 
       "../Psybrus/External/imgui", 
       "../Psybrus/External/jsoncpp/include", 
    }

    -- Platform includes.
    configuration "html5-clang-asmjs"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/HTML5/",
       }

    configuration "linux-*"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/Linux/",
       }

    configuration "osx-*"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/OSX/",
       }

    configuration "android-*"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/Android/",
       }

    configuration "windows-*"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/Windows/"
       }

    configuration "winphone-*"
       includedirs {
          "../Psybrus/Engine/Source/Platforms/Windows/"
       }

    configuration "*"
end

-- Common project setup.
function PsyProjectCommon( _name, _lang )
	project( _name )

	-- Common flags for all configurations.
	configuration "*"
		flags { "FloatFast", "NativeWChar" }

	configuration "windows-*"
		flags { "StaticRuntime" }

	-- Windows config defines + flags.
	configuration "windows-*"
		defines { "WINDOWS", "_WIN32", "WIN32" }

	-- Windows config defines + flags.
	configuration "winphone-*"
		defines { "WINPHONE" }

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
	--configuration { "Debug", "gmake" }
	--	linkoptions { "-rdynamic" }

	--configuration { "Release", "gmake" }
	--	linkoptions { "-rdynamic" }

	-- Common defines for build targets across all types of project.
	configuration "Debug"
		defines { "_DEBUG", "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Symbols" }

	configuration "Production"
		defines { "NDEBUG" }
		flags { "Symbols" }

	-- Optimised builds.
	configuration { "windows-* or linux-* or android-* or osx-*", "Release" }
		flags { "Optimize" }

	configuration { "windows-* or linux-* or android-* or osx-*", "Production" }
		flags { "Optimize" }

	configuration { "Debug", "vs*"}
		defines { "_ITERATOR_DEBUG_LEVEL=1" }

	-- Setup language specific support.
	languageOptions = {
		[ "C" ] = "C",
		[ "C++" ] = "C++",
		[ "C++11" ] = "C++",
		[ "C++14" ] = "C++",
		[ "C++17" ] = "C++"
	}

	gccLanguageOptions = {
		[ "C" ] = {},
		[ "C++" ] = {},
		[ "C++11" ] = { "-std=c++11" },
		[ "C++14" ] = { "-std=c++11" }, -- TODO: C++14.
		[ "C++17" ] = { "-std=c++11" }  -- TODO: C++17.
	}

	clangLanguageOptions = {
		[ "C" ] = {},
		[ "C++" ] = { "-stdlib=libc++" },
		[ "C++11" ] = { "-stdlib=libc++", "-std=c++11" },
		[ "C++14" ] = { "-stdlib=libc++", "-std=c++14" },
		[ "C++17" ] = { "-stdlib=libc++", "-std=c++17" }
	}

	configuration "*"
		language( languageOptions[ _lang ] )

	configuration "*-gcc or *-gcc-*"
		buildoptions( gccLanguageOptions[ _lang ] )

	configuration "*-clang or html5-clang-*"
		buildoptions( clangLanguageOptions[ _lang ] )

	-- Terminate project.
	configuration "*"
end

-- Common engine project.
function PsyProjectCommonEngine( _name )
	PsyProjectCommon( _name, "C++14" )

	configuration { "gmake", "linux-* or osx-*" }
		links {
			"c++"
		}

	configuration { "gmake", "android-*" }
		links {
			--"c++_shared"
		}

	-- Extra warnings + fatal warnings.
	configuration "vs*"
		flags { "ExtraWarnings" }

	-- Defines for all configurations
	configuration "Debug"
		defines { "PSY_USE_PROFILER=1" }
		defines { "PSY_DEBUG" }

	configuration "Release"
		defines { "PSY_USE_PROFILER=1" }
		defines { "PSY_RELEASE" }

	configuration "Production"
		defines { "PSY_USE_PROFILER=0" }
		defines { "PSY_PRODUCTION" }

	-- Import pipeline.
	configuration "windows-* or linux-* or osx-*"
		defines { "PSY_IMPORT_PIPELINE" }

	-- Add default include paths.
	configuration( "*" )
		includedirs {
			"../../External/imgui"
		}

	configuration( "windows-* or winphone-*" )
		includedirs {
			"./Platforms/Windows/",
		}

	configuration( "linux-*" )
		includedirs {
			"./Platforms/Linux/",
		}

	configuration( "osx-*" )
		includedirs {
			"./Platforms/OSX/",
		}

	configuration( "android-*" )
		includedirs {
			"./Platforms/Android/",
		}

	configuration( "html5-clang-asmjs" )
		includedirs {
			"./Platforms/HTML5/",
			"$(EMSCRIPTEN)/system/lib/libcxxabi/include",
		}

	-- Build defines for gmake
	configuration "gmake"
		defines {
			"BUILD_ACTION=\\\"" .. _ACTION .. "\\\"",
			"BUILD_TOOLCHAIN=\\\"" .. _OPTIONS[ "toolchain" ] .. "\\\"",
		}

	configuration { "gmake", "Debug" }
		defines {
			"BUILD_CONFIG=\\\"Debug\\\"",
		}

	configuration { "gmake", "Release" }
		defines {
			"BUILD_CONFIG=\\\"Release\\\"",
		}

	configuration { "gmake", "Production" }
		defines {
			"BUILD_CONFIG=\\\"Production\\\"",
		}

	-- Build defines for vs
	configuration "vs*"
		defines {
			"BUILD_ACTION=\"" .. _ACTION .. "\"",
			"BUILD_TOOLCHAIN=\"" .. _OPTIONS[ "toolchain" ] .. "\"",
		}

	configuration { "vs*", "Debug" }
		defines {
			"BUILD_CONFIG=\"Debug\"",
		}

	configuration { "vs*", "Release" }
		defines {
			"BUILD_CONFIG=\"Release\"",
		}

	configuration { "vs*", "Production" }
		defines {
			"BUILD_CONFIG=\"Production\"",
		}

	-- Terminate project.
	configuration "*"
end


-- Setup psybrus exe project.
function PsyProjectPsybrusExe( _name, _exeName )
	LLVMcxxabiProject()

	group( _name )

	PsyProjectCommonEngine( _exeName )
	PsyPlatformIncludes()

	configuration "html5-* or linux-* or osx-* or android-*"
		prebuildcommands {
			"python ../../Psybrus/reflection_parse.py " .. solution().name
		}

	-- Setup android project (if it is one).
	SetupAndroidProject()

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	local targetNamePrefix = _exeName
	configuration "*"
		targetname( targetNamePrefix .. "-" )

	-- OSX specifics.
	configuration { "osx-*", "Debug" }
		targetname( targetNamePrefix .. "-Debug" )
		targetsuffix ""
	configuration { "osx-*", "Release" }
		targetname( targetNamePrefix .. "-Release" )
		targetsuffix ""
	configuration { "osx-*", "Production" }
		targetname( targetNamePrefix .. "-Production" )
		targetsuffix ""
	
	--
	configuration "windows-* or linux-* or osx-*"
		targetdir ( "../Dist" )

    debugdir "../Dist"

	configuration "*"
		links {
			_name .. "Lib"
		}

    configuration "*"
	    PsyAddEngineLinks {
	       "Engine",
	       "System_Sound",
	       "System_Scene",
	       "System_Renderer",
	       "System_Os",
	       "System_Network",
	       "System_Debug",
	       "System_Content",
	       "System_File",
	       "System",
	       "Serialisation",
	       "Reflection",
	       "Math",
	       "Import",
	       "Events",
	       "Base",
	    }
	   
	    PsyAddExternalLinks {
	       "BulletPhysics",
	       "freetype",
	       "imgui",
	       "jsoncpp",
	       "libb64",
	       "png",
	       "rg-etc1",
	       "squish",
	       "SoLoud",
	       "zlib",
	    }

	PsyAddSystemLibs()

    configuration { "windows-* or linux-* or osx-*" }
       PsyAddExternalLinks {
          "assimp",
          "assimp_contrib",
          "fcpp",
          "glslang",
          "glsl-optimizer",
          "glew",
          "ThinkGear",
       }

    configuration { "windows-* or linux-* or osx-* or android-*" }
       PsyAddExternalLinks {
          "RakNet",
          "webby",
       }

	-- asmjs post build.
	configuration { "html5-clang-asmjs", "Debug" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) mkdir -p ./PackedContent",
			"$(SILENT) cp ../../Dist/PackedContent/html5/* ./PackedContent",
			"$(SILENT) echo Running asmjs finalise \\(Debug\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O0 --emrun --memory-init-file 1 --js-opts 0 -g3 -s ASM_JS=1 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=" .. GAME.html5.total_memory .. " \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	configuration { "html5-clang-asmjs", "Release" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) mkdir -p ./PackedContent",
			"$(SILENT) cp ../../Dist/PackedContent/html5/* ./PackedContent",
			"$(SILENT) echo Running asmjs finalise \\(Release\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O3 --emrun --memory-init-file 1 --js-opts 1 -g3 -s ASM_JS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=" .. GAME.html5.total_memory .. " \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

	configuration { "html5-clang-asmjs", "Production" }
		postbuildcommands {
			"$(SILENT) echo Copying packed content.",
			"$(SILENT) mkdir -p ./PackedContent",
			"$(SILENT) cp ../../Dist/PackedContent/html5/* ./PackedContent",
			"$(SILENT) echo Running asmjs finalise \\(Production\\)",
			"$(SILENT) mv $(TARGET) $(TARGET).o",
			"$(SILENT) $(EMSCRIPTEN)/emcc -v -O3 --memory-init-file 1 --js-opts 1 --llvm-lto 1 -s ASM_JS=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=" .. GAME.html5.total_memory .. " \"$(TARGET).o\" -o \"$(TARGET)\".html --preload-file ./PackedContent@/PackedContent",
		}

   configuration "windows-*"
      includedirs {
         "../Psybrus/Engine/Source/Platforms/Windows/"
      }

	-- Terminate project.
	configuration "*"
end

-- Setup game exe project.
function PsyProjectGameLib( _name )
	group( _name )
	libName = _name .. "Lib"

	PsyProjectCommonEngine( libName )
	PsyPlatformIncludes()
	print( "Adding Game Library: " .. libName )

	configuration "*"
		kind "StaticLib"
		language "C++"

	-- Add STATICLIB define for libraries.
	configuration "*"
		defines{ "STATICLIB" }

	configuration "html5-* or linux-* or osx-* or android-*"
		prebuildcommands {
			"python ../../Psybrus/reflection_parse.py " .. solution().name
		}

	configuration "windows-*"
		prebuildcommands {
			"python.exe ../../Psybrus/reflection_parse.py " .. solution().name
		}

	-- Terminate project.
	configuration "*"
end

-- Setup game exe project.
function PsyProjectGameExe( _name )
	PsyProjectPsybrusExe( _name, _name )
	print( "Adding Game Executable: " .. _name )

	configuration "html5-* or linux-* or osx-* or windows-* or winphone-*"
		kind "WindowedApp"
		language "C++"

	configuration "*"
		files { 
			"../Psybrus/Engine/Targets/Game.cpp" 
		}

	-- Add natvis for VS.
	configuration "vs2015"
		files( "../Psybrus/Tools/VsVisualiser/Psybrus.natvis" )

	configuration "*"
end

-- Setup game exe project.
function PsyProjectImporterExe( _name )
	-- TODO: Set a boolean in the toolchain.
	if string.match( _OPTIONS[ "toolchain" ], "windows-.*" ) or
	   string.match( _OPTIONS[ "toolchain" ], "linux-.*" ) or
	   string.match( _OPTIONS[ "toolchain" ], "osx-.*" ) then
		PsyProjectPsybrusExe( _name, "Importer" )
		print( "Adding Importer Executable: " .. "Importer" )

		configuration "*"
			kind "ConsoleApp"
			language "C++"

			files { 
				"../Psybrus/Engine/Targets/Importer.cpp" 
			}

		-- Add natvis for VS.
		configuration "vs2015"
			files( "../Psybrus/Tools/VsVisualiser/Psybrus.natvis" )

		configuration "*"
	end
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

	-- Add natvis for VS.
	configuration "vs2015"
		files( "../Psybrus/Tools/VsVisualiser/Psybrus.natvis" )

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

	configuration "Debug"
		flags { "DebugRuntime", "Optimize"}

	configuration "Release or Profile or Production"
		flags { "ReleaseRuntime", "Optimize"}

	-- External librarys should be built with no WinRT language extensions.
	--configuration "winphone-*"
	--	flags { "NoWinRTExtensions" }

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

