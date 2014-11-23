-- Toolchain setup.
function PsySetupToolchain()
	--
	-- Based on bx's toolchain.lua.
	-- Copyright 2010-2014 Branimir Karadzic. All rights reserved.
	-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
	--
	if _ACTION == "gmake" then
		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				-- Linux targets
				{ "linux-gcc",			"Linux (GCC compiler)"			},
				{ "linux-clang",		"Linux (Clang compiler)"		},

				-- Windows targets: Experimental cross compilation.
				{ "windows-mingw-gcc",	"Windows (mingw GCC compiler)"	},
				
				-- asm.js targets: Experimental JS compilation.
				{ "asmjs",				"Emscripten/asm.js"				},
			},
		}
	
		-- Linux gcc.
		if _OPTIONS[ "toolchain" ] == "linux-gcc" then
			premake.gcc.cc = "gcc"
			premake.gcc.cxx = "g++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-gcc" )
		end

		-- Linux clang.
		if _OPTIONS[ "toolchain" ] == "linux-clang" then
			premake.gcc.cc = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-clang" )
		end

		-- Windows mingw gcc.
		if _OPTIONS[ "toolchain" ] == "windows-mingw-gcc" then
			premake.gcc.cc = "x86_64-w64-mingw32-gcc"
			premake.gcc.cxx = "x86_64-w64-mingw32-g++"
			premake.gcc.ar = "x86_64-w64-mingw32-ar"
			location ( "Projects/" .. _ACTION .. "-windows-mingw-gcc" )
		end

		-- asmjs.
		if _OPTIONS[ "toolchain" ] == "asmjs" then
			premake.gcc.cc = "$(EMSCRIPTEN)/emcc"
			premake.gcc.cxx = "$(EMSCRIPTEN)/em++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-asmjs" )
		end

		-- Configurations
		targetdir ( "Build/" .. _ACTION .. "/bin" )
		objdir ( "Build/" .. _ACTION .. "/obj" )

		configuration { "linux-gcc", "x32" }
			targetdir ( "Build/" .. _ACTION .. "-linux32-gcc/bin" )
			objdir ( "Build/" .. _ACTION .. "-linux32-gcc/obj" )
			buildoptions { "-m32" }

		configuration { "linux-gcc", "x64" }
			targetdir ( "Build/" .. _ACTION .. "-linux64-gcc/bin" )
			objdir ( "Build/" .. _ACTION .. "-linux64-gcc/obj" )
			buildoptions { "-m64" }

		configuration { "linux-clang", "x32" }
			targetdir ( "Build/" .. _ACTION .. "-linux32-clang/bin" )
			objdir ( "Build/" .. _ACTION .. "-linux32-clang/obj" )
			buildoptions { "-m32" }

		configuration { "linux-clang", "x64" }
			targetdir ( "Build/" .. _ACTION .. "-linux64-clang/bin" )
			objdir ( "Build/" .. _ACTION .. "-linux64-clang/obj" )
			buildoptions { "-m64" }

		configuration { "windows-mingw-gcc", "x32" }
			targetdir ( "Build/" .. _ACTION .. "-windows32-mingw-gcc/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows32-mingw-gcc/obj" )
			buildoptions { "-m32" }

		configuration { "windows-mingw-gcc", "x64" }
			targetdir ( "Build/" .. _ACTION .. "-windows64-mingw-gcc/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows64-mingw-gcc/obj" )
			buildoptions { "-m64" }

		configuration { "asmjs" }
			targetdir ( "Build/" .. _ACTION .. "-asmjs/bin" )
			objdir ( "Build/" .. _ACTION .. "-asmjs/obj" )

	end

	if _ACTION == "vs2012" or _ACTION == "vs2013" then
		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				{ "windows-vs-v110",		"Windows (VS2012 compiler)" },
				{ "windows-vs-v120",		"Windows (VS2013 compiler)" },
				{ "windows-vs-clang",		"Windows (Clang)" },
			},
		}

		-- win-vs-v110
		if _OPTIONS[ "toolchain" ] == "windows-vs-v110" then
			premake.vstudio.toolset = "v110"
			location ( "Build/" .. _ACTION .. "windows-vs-v110" )
		end

		-- win-vs-v120
		if _OPTIONS[ "toolchain" ] == "windows-vs-v120" then
			premake.vstudio.toolset = "v120"
			location ( "Build/" .. _ACTION .. "windows-vs-v120" )
		end

		-- win-vs-clang
		if _OPTIONS[ "toolchain" ] == "windows-vs-clang" then
			premake.vstudio.toolset = "LLVM-" .. _ACTION
			location ( "Build/" .. _ACTION .. "windows-vs-clang" )
		end

		-- Configurations
		configuration { "windows-vs-v110" }
			targetdir ( "Build/" .. _ACTION .. "windows-vs-v110/bin" )
			objdir ( "Build/" .. _ACTION .. "windows-vs-v110/obj" )

		configuration { "windows-vs-v120" }
			targetdir ( "Build/" .. _ACTION .. "windows-vs-v120/bin" )
			objdir ( "Build/" .. _ACTION .. "windows-vs-v120/obj" )

		configuration { "windows-vs-clang" }
			targetdir ( "Build/" .. _ACTION .. "windows-vs-clang/bin" )
			objdir ( "Build/" .. _ACTION .. "windows-vs-clang/obj" )
	end

	-- Check we got a valid toolchain
	if _OPTIONS[ "toolchain" ] == "invalid" or
	   _OPTIONS[ "toolchain" ] == nil then
		print "Toolchain must be specified"
		os.exit(1)
	end

	-- target suffix.
	configuration "Debug"
		targetsuffix "Debug"

	configuration "Release"
		targetsuffix "Release"

	configuration "Profile"
		targetsuffix "Profile"

	configuration "Production"
		targetsuffix "Production"

	configuration { "*" }

end
