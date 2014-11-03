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
				{ "linux-gcc",     "Linux (GCC compiler)"   },
				{ "linux-clang",   "Linux (Clang compiler)" },
				{ "asmjs",         "Emscripten/asm.js"      },
			},
		}
	
		-- Linux gcc.
		if _OPTIONS[ "toolchain" ] == "linux-gcc" then
			premake.gcc.cc = "gcc"
			premake.gcc.cxx = "g++"
			premake.gcc.ar = "ar"
		end

		-- Linux clang.
		if _OPTIONS[ "toolchain" ] == "linux-clang" then
			premake.gcc.cc = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar = "ar"
		end

		-- Linux asmjs.
		if _OPTIONS[ "toolchain" ] == "asmjs" then
			premake.gcc.cc = "emcc"
			premake.gcc.cxx = "em++"
			premake.gcc.ar = "ar"
			configuration "*"
				includedirs {
					"/home/neilo/Dev/boost_emscripten"
				}
		end
	end

	-- asmjs post build.
	configuration { "asmjs" }
		postbuildcommands {
			"$(SILENT) echo Running asmjs finalise.",
			"$(SILENT) emcc -O2 -s TOTAL_MEMORY=268435456 \"$(TARGET)\" -o \"$(TARGET)\".html"
		}

end
