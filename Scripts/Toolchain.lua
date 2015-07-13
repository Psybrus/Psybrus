ANDROID_NDK_VERSION = "21"
ANDROID_SDK_VERSION = "22"

function IsHostOS( _os )
	return _OS == _os
end

function IsTargetOS( _os )
	return os.is( _os ) 
end

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
				{ "linux-clang",		"Linux (Clang 3.5 compiler)"	},

				-- OSX targets
				{ "osx-clang",			"OSX (Clang compiler)"		},

				-- Windows targets: Experimental cross compilation.
				{ "windows-mingw-gcc",	"Windows (mingw GCC compiler)"	},
				
				-- asm.js targets: Experimental JS compilation.
				{ "asmjs",				"Emscripten/asm.js"				},

				-- Android targets: Experimental.
				{ "android-clang-arm", "Android ARM (Clang 3.5 compiler)" },
				{ "android-gcc-arm", "Android ARM (GCC 4.9 compiler)" },
			},
		}

			
		-- Linux gcc.
		if _OPTIONS[ "toolchain" ] == "linux-gcc" then
			premake.gcc.cc = "ccache gcc"
			premake.gcc.cxx = "ccache g++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-gcc" )
		end

		-- Linux clang.
		if _OPTIONS[ "toolchain" ] == "linux-clang" then
			-- temporarily disabled ccache, issues with teamcity.
			premake.gcc.cc = "clang-3.5 -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.cxx = "clang++-3.5 -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-clang" )
		end

		-- OSX clang.
		if _OPTIONS[ "toolchain" ] == "osx-clang" then
			premake.gcc.cc = "clang -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.cxx = "clang -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.ar = "ar"
						
			location ( "Projects/" .. _ACTION .. "-osx-clang" )
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
			-- todo: try ccache.
			premake.gcc.cc = "$(EMSCRIPTEN)/emcc"
			premake.gcc.cxx = "$(EMSCRIPTEN)/em++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-asmjs" )
		end

		-- android-clang-arm
		if _OPTIONS[ "toolchain" ] == "android-clang-arm" then
			local sdkVersion = "android-" .. ANDROID_NDK_VERSION

			premake.gcc.llvm = true
			premake.gcc.cc = "$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/bin/clang --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
			premake.gcc.cxx = "$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/bin/clang++ -B $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/arm-linux-androideabi/bin --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
			premake.gcc.ar = "$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/bin/llvm-ar"
			location ( "Projects/" .. _ACTION .. "-android-clang-arm" )

			buildoptions { 
				"-target armv7-none-linux-androideabi",
				"-march=armv7-a",
				"-mfpu=vfp",
				"-mhard-float",
				--"-mfloat-abi=hard",
				--"-nostdinc",

				-- Disable as it causes noise that we can't fix just yet.
				"-Wno-macro-redefined"
			}

			linkoptions { 
				"-target armv7-none-linux-androideabi",
				"-Wl,--fix-cortex-a8",
				"-Wl,--no-warn-mismatch",
				"-nostdlib",
			}

			configuration( "*" )
				defines {
					"_NDK_MATH_NO_SOFTFP"
				}

				links {
					"c",
					"m",
					"dl"
				}

				defines {
					"ANDROID_SDK_VERSION=" .. ANDROID_SDK_VERSION,
					"ANDROID_NDK_VERSION=" .. ANDROID_NDK_VERSION
				}

				local useStdCpp = true
				local useLibCpp = false

				-- GNU stdc++
				if useStdCpp then
					links {
						"gnustl_static"
					}

					includedirs {
						"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include",
						"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include"
					}

					libdirs {
						"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a",
						"$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib/gcc/arm-linux-androideabi/4.9"
					}
				end


				-- LLVM libc++
				if useLibCpp then
					links {
						"gabi++_shared",
						"c++_shared"
					}

					includedirs {
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libcxx/include"
					}

					libdirs {
						"$(ANDROID_NDK)/sources/cxx-stl/gabi++/libs/armeabi-v7a",
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
					}
				end

				includedirs {
					"$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/lib/clang/3.5/include",
					"$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm/usr/include",
					"$(ANDROID_NDK)/sources/android/support/include",
				}


				includedirs {
					"$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/lib/clang/3.5/include",
					"$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm/usr/include",
					"$(ANDROID_NDK)/sources/android/support/include",
				}

				libdirs {
					"$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm/usr/lib",
			}

		end

		-- android-clang-arm
		if _OPTIONS[ "toolchain" ] == "android-gcc-arm" then
			local sdkVersion = "android-" .. ANDROID_NDK_VERSION

			if IsHostOS("windows") then
				premake.gcc.llvm = true
				premake.gcc.cc = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/windows-x86_64/bin/arm-linux-androideabi-gcc.exe --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.cxx = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/windows-x86_64/bin/arm-linux-androideabi-g++.exe --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.ar = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/windows-x86_64/bin/arm-linux-androideabi-ar.exe"
			elseif IsHostOS("linux") then
				premake.gcc.llvm = true
				premake.gcc.cc = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.cxx = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++ --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.ar = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar"
			elseif IsHostOS("macosx") then
				premake.gcc.llvm = true
				premake.gcc.cc = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.cxx = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-g++ --sysroot=$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
				premake.gcc.ar = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-ar"
			else
				print "Toolchain does not exist for host OS"
				os.exit(1)
			end
			location ( "Projects/" .. _ACTION .. "-android-gcc-arm" )

			buildoptions { 
				"-march=armv7-a",
				"-mfpu=vfp",
				"-mfloat-abi=softfp"
			}

			linkoptions { 
				"-Wl,--fix-cortex-a8",
				--"-Wl,--no-warn-mismatch",
			}

			-- Add default include paths.
			configuration( "*" )
				defines {
					"_NDK_MATH_NO_SOFTFP=1"
				}

				links {
					"c",
					"m",
					"dl",
					"android",
					"log"
				}

				defines {
					"ANDROID_SDK_VERSION=" .. ANDROID_SDK_VERSION,
					"ANDROID_NDK_VERSION=" .. ANDROID_NDK_VERSION
				}

				local useStdCpp = false
				local useLibCpp = true

				-- GNU stdc++
				if useStdCpp then
					links {
						"gnustl_static"
					}

					includedirs {
						"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include",
						"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include"
					}

					if IsHostOS("windows") then
						libdirs {
							"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a",
							"$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/windows-x86_64/lib/gcc/arm-linux-androideabi/4.9"
						}
					else
						libdirs {
							"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a",
							"$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib/gcc/arm-linux-androideabi/4.9"
						}
					end
				end


				-- LLVM libc++
				if useLibCpp then
					links {
						"c++_static",
					}

					includedirs {
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++abi/libcxxabi/include",
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libcxx/include",
					}

					libdirs {
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++abi/libs/armeabi-v7a",
						"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
					}
				end

				includedirs {
					"$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/include",
					"$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm/usr/include",
					"$(ANDROID_NDK)/sources/android/support/include",
				}

				libdirs {
					"$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib",
					"$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm/usr/lib",
			}

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

		configuration { "android-clang-arm" }
			targetdir ( "Build/" .. _ACTION .. "-android-clang-arm/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-clang-arm/obj" )

		configuration { "android-gcc-arm" }
			targetdir ( "Build/" .. _ACTION .. "-android-gcc-arm/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-gcc-arm/obj" )
	end

	-- Experimental.
	if _ACTION == "ndk-makefile" then
		local sdkVersion = "android-" .. ANDROID_NDK_VERSION

		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				-- Android targets: Experimental.
				{ "android-clang-arm", "Android ARM (Clang 3.5 compiler)" },
			},
		}

		-- android-clang-arm
		if _OPTIONS[ "toolchain" ] == "android-clang-arm" then
			location ( "Projects/" .. _ACTION .. "-android-clang-arm" )

			includedirs {
				"$(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/lib/clang/3.5/include",
				"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libcxx/include",
				"$(ANDROID_NDK)/sources/android/support/include",
			}
		end

		-- Configurations
		targetdir ( "Build/" .. _ACTION .. "/bin" )
		objdir ( "Build/" .. _ACTION .. "/obj" )

		configuration { "android-clang-arm" }
			targetdir ( "Build/" .. _ACTION .. "-android-clang-arm/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-clang-arm/obj" )
	end

	if _ACTION == "xcode3" or _ACTION == "xcode4" then
		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				-- OSX targets
				{ "osx-clang",			"OSX (Clang compiler)"		},
			}
		}

		-- Configurations
		targetdir ( "Build/" .. _ACTION .. "/bin" )
		objdir ( "Build/" .. _ACTION .. "/obj" )

		location ( "Projects/" .. _ACTION .. "-osx64-clang" )

		configuration { "osx-clang", "x64" }
			targetdir ( "Build/" .. _ACTION .. "-osx64-clang/bin" )
			objdir ( "Build/" .. _ACTION .. "-osx64-clang/obj" )
			buildoptions { "-m64" }

	end


	if _ACTION == "vs2012" or _ACTION == "vs2013" or _ACTION == "vs2015" then
		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				{ "windows-vs-v110",		"Windows (VS2012 compiler)" },
				{ "windows-vs-v120",		"Windows (VS2013 compiler)" },
				{ "windows-vs-v140",		"Windows (VS2015 compiler)" },
				{ "windows-vs-clang",		"Windows (Clang)" },
			},
		}

		-- win-vs-v110
		if _OPTIONS[ "toolchain" ] == "windows-vs-v110" then
			premake.vstudio.toolset = "v110"
			location ( "Projects/" .. _ACTION .. "-windows-vs-v110" )
		end

		-- win-vs-v120
		if _OPTIONS[ "toolchain" ] == "windows-vs-v120" then
			premake.vstudio.toolset = "v120"
			location ( "Projects/" .. _ACTION .. "-windows-vs-v120" )
		end

		-- win-vs-v130
		if _OPTIONS[ "toolchain" ] == "windows-vs-v140" then
			premake.vstudio.toolset = "v140"
			location ( "Projects/" .. _ACTION .. "-windows-vs-v140" )
		end
				-- win-vs-clang
		if _OPTIONS[ "toolchain" ] == "windows-vs-clang" then
			premake.vstudio.toolset = "LLVM-" .. _ACTION
			location ( "Projects/" .. _ACTION .. "-windows-vs-clang" )
		end

		-- Configurations
		configuration { "windows-vs-v110" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-v110/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-v110/obj" )

		configuration { "windows-vs-v120" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-v120/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-v120/obj" )

		configuration { "windows-vs-v140" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-v140/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-v140/obj" )

		configuration { "windows-vs-clang" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-clang/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-clang/obj" )
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
