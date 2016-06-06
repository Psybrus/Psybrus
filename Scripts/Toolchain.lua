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
				{ "linux-gcc",			"Linux (GCC compiler)"				},
				{ "linux-clang",		"Linux (Clang 3.7 compiler)"		},

				-- OSX targets
				{ "osx-clang",			"OSX (Clang compiler)"				},

				-- HTML5 targets
				{ "html5-clang-asmjs",	"HTML5 asm.js (Clang compiler)"		},

				-- Android targets
				{ "android-gcc-arm", 	"Android ARM (GCC 4.9 compiler)"	},
				{ "android-gcc-x86", 	"Android x86 (GCC 4.9 compiler)"	},
				{ "android-clang-arm", 	"Android ARM (Clang compiler)"		},
				{ "android-clang-x86", 	"Android x86 (Clang compiler)"		},
			},
		}

			
		-- Linux gcc.
		if _OPTIONS[ "toolchain" ] == "linux-gcc" then
			premake.gcc.cc = "ccache gcc"
			premake.gcc.cxx = "ccache g++"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-gcc" )

			linkoptions {
				"-Wl,-export-dynamic"
			}
		end

		-- Linux clang.
		if _OPTIONS[ "toolchain" ] == "linux-clang" then
			-- temporarily disabled ccache, issues with teamcity.
			premake.gcc.cc = "clang-3.7 -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.cxx = "clang++-3.7 -Qunused-arguments -fcolor-diagnostics"
			premake.gcc.ar = "ar"
			location ( "Projects/" .. _ACTION .. "-linux-clang" )

			linkoptions {
				"-Wl,-export-dynamic"
			}
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
		if _OPTIONS[ "toolchain" ] == "html5-clang-asmjs" then
			-- todo: try ccache.
			premake.gcc.llvm = true
			premake.gcc.cc = "$(EMSCRIPTEN)/emcc"
			premake.gcc.cxx = "$(EMSCRIPTEN)/em++"
			premake.gcc.ar = "$(EMSCRIPTEN)/emar"
			location ( "Projects/" .. _ACTION .. "-html5-clang-asmjs" )
		end

		function SetupAndroidCompiler(toolchainPath, sysRoot, c_compiler, cpp_compiler, linker)
			if IsHostOS("windows") then
				premake.gcc.llvm = true
				premake.gcc.cc = toolchainPath .. "/windows-x86_64/bin/" .. c_compiler .. ".exe --sysroot=" .. sysRoot
				premake.gcc.cxx = toolchainPath .. "/windows-x86_64/bin/" .. cpp_compiler .. ".exe --sysroot=" .. sysRoot
				premake.gcc.ar = toolchainPath .. "/windows-x86_64/bin/" .. linker .. ".exe"
			elseif IsHostOS("linux") then
				premake.gcc.llvm = true
				premake.gcc.cc = toolchainPath .. "/linux-x86_64/bin/" .. c_compiler .. " --sysroot=" .. sysRoot
				premake.gcc.cxx = toolchainPath .. "/linux-x86_64/bin/" .. cpp_compiler .. " --sysroot=" .. sysRoot
				premake.gcc.ar = toolchainPath .. "/linux-x86_64/bin/" .. linker
			elseif IsHostOS("macosx") then
				premake.gcc.llvm = true
				premake.gcc.cc = toolchainPath .. "/darwin-x86_64/bin/" .. c_compiler .. " --sysroot=" .. sysRoot
				premake.gcc.cxx = toolchainPath .. "/darwin-x86_64/bin/" .. cpp_compiler .. " --sysroot=" .. sysRoot
				premake.gcc.ar = toolchainPath .. "/darwin-x86_64/bin/" .. linker
			else
				print "Toolchain does not exist for host OS"
				os.exit(1)
			end

			defines {
				"ANDROID_SDK_VERSION=" .. GAME.android.sdk_version,
				"ANDROID_NDK_VERSION=" .. GAME.android.ndk_version
			}

			configuration( "*" )
				links {
					"c",
					"m",
					"dl",
					"android",
					"log"
				}

				-- LLVM libc++
				links {
					"c++_static",
				}

				includedirs {
					"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++abi/libcxxabi/include",
					"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libcxx/include",
				}

				libdirs {
					"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++abi/libs/x86",
					"$(ANDROID_NDK)/sources/cxx-stl/llvm-libc++/libs/x86",
				}

				includedirs {
					toolchainPath .. "/linux-x86_64/include",
					sysRoot .. "/arch-x86/usr/include",
					"$(ANDROID_NDK)/sources/android/support/include",
				}

				libdirs {
					toolchainPath .. "/linux-x86_64/lib",
					sysRoot .. "/usr/lib",
				}
		end

		-- GCC 4.9
		if _OPTIONS[ "toolchain" ] == "android-gcc-arm" then
			local sdkVersion = "android-" .. GAME.android.ndk_version

			toolchainPath = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt"
			sysRoot = "$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
			SetupAndroidCompiler( toolchainPath, sysRoot, "arm-linux-androideabi-gcc", "arm-linux-androideabi-g++", "arm-linux-androideabi-ar" )
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
		end

		if _OPTIONS[ "toolchain" ] == "android-gcc-x86" then
			local sdkVersion = "android-" .. GAME.android.ndk_version

			toolchainPath = "$(ANDROID_NDK)/toolchains/x86-4.9/prebuilt"
			sysRoot = "$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-x86"
			SetupAndroidCompiler( toolchainPath, sysRoot, "i686-linux-android-gcc", "i686-linux-android-g++", "i686-linux-android-ar" )
			location ( "Projects/" .. _ACTION .. "-android-gcc-x86" )
		end


		-- Clang 3.6
		if _OPTIONS[ "toolchain" ] == "android-clang-arm" then
			local sdkVersion = "android-" .. GAME.android.ndk_version

			toolchainPath = "$(ANDROID_NDK)/toolchains/llvm-3.6/prebuilt"
			sysRoot = "$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-arm"
			SetupAndroidCompiler( toolchainPath, sysRoot, "clang", "clang++", "ld" )

			location ( "Projects/" .. _ACTION .. "-android-clang-arm" )

			buildoptions { 
				"-arch arm",
				--"-mfpu=vfp",
				--"-mfloat-abi=softfp"
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
		end

		if _OPTIONS[ "toolchain" ] == "android-clang-x86" then
			local sdkVersion = "android-" .. GAME.android.ndk_version

			toolchainPath = "$(ANDROID_NDK)/toolchains/llvm-3.6/prebuilt"
			sysRoot = "$(ANDROID_NDK)/platforms/" .. sdkVersion .. "/arch-x86"
			SetupAndroidCompiler( toolchainPath, sysRoot, "clang", "clang++", "ld" )
			location ( "Projects/" .. _ACTION .. "-android-clang-x86" )
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

		configuration { "html5-clang-asmjs" }
			targetdir ( "Build/" .. _ACTION .. "-html5-clang-asmjs/bin" )
			objdir ( "Build/" .. _ACTION .. "-html5-clang-asmjs/obj" )

		configuration { "android-gcc-arm" }
			targetdir ( "Build/" .. _ACTION .. "-android-gcc-arm/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-gcc-arm/obj" )

		configuration { "android-gcc-x86" }
			targetdir ( "Build/" .. _ACTION .. "-android-gcc-x86/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-gcc-x86/obj" )

		configuration { "android-clang-arm" }
			targetdir ( "Build/" .. _ACTION .. "-android-gcc-arm/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-gcc-arm/obj" )

		configuration { "android-clang-x86" }
			targetdir ( "Build/" .. _ACTION .. "-android-gcc-x86/bin" )
			objdir ( "Build/" .. _ACTION .. "-android-gcc-x86/obj" )
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


	if _ACTION == "vs2013" or _ACTION == "vs2015" then
		newoption {
			trigger = "toolchain",
			value = "toolchain",
			description = "Choose toolchain",
			allowed = {
				{ "windows-vs-v120",		"Windows (VS2013 compiler)" },
				{ "windows-vs-v140",		"Windows (VS2015 compiler)" },
				{ "winphone-vs-v120_wp81",	"Windows Phone (8.1)"}
			},
		}

		newoption {
			trigger = "with-dx12",
			description = "Build with DirectX 12"
		}

		newoption {
			trigger = "with-vk",
			description = "Build with Vulkan"
		}

		if _OPTIONS[ "with-dx12" ] then
			windowstargetplatformversion "10.0.10240.0"
		end

		-- win-vs-v120
		if _OPTIONS[ "toolchain" ] == "windows-vs-v120" then
			premake.vstudio.toolset = "v120"
			location ( "Projects/" .. _ACTION .. "-windows-vs-v120" )
		end

		-- win-vs-v140
		if _OPTIONS[ "toolchain" ] == "windows-vs-v140" then
			premake.vstudio.toolset = "v140"
			location ( "Projects/" .. _ACTION .. "-windows-vs-v140" )
		end

		-- win-vs-v140
		if _OPTIONS[ "toolchain" ] == "winphone-vs-v120_wp81" then
			premake.vstudio.toolset = "v120_wp81"
			premake.vstudio.storeapp = "8.1"
			premake.vstudio.splashpath = "..\\..\\Dist\\WinPhone\\SplashScreen.scale-240.png"
			location ( "Projects/" .. _ACTION .. "-winphone-vs-v120_wp81" )
		end

		-- Configurations
		configuration { "windows-vs-v120" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-v120/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-v120/obj" )

		configuration { "windows-vs-v140" }
			targetdir ( "Build/" .. _ACTION .. "-windows-vs-v140/bin" )
			objdir ( "Build/" .. _ACTION .. "-windows-vs-v140/obj" )

		configuration { "winphone-vs-v120_wp81" }
			targetdir ( "Build/" .. _ACTION .. "-winphone-vs-v120_wp81/bin" )
			objdir ( "Build/" .. _ACTION .. "-winphone-vs-v120_wp81/obj" )
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

	configuration "Production"
		targetsuffix "Production"

	configuration { "*" }

end
