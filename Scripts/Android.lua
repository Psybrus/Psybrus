dofile( "NDK/ndk.lua" )
dofile( "NDK/common.lua" )
dofile( "NDK/makefile.lua" )
dofile( "NDK/manifest.lua" )

ANDROID_TOOL="$(ANDROID_SDK)/tools/android"	
if IsHostOS("windows") then
	ANDROID_TOOL="\"$(ANDROID_SDK)/tools/android\""
end


ANDROID_NDK_PATH = os.getenv("ANDROID_NDK")

function mkdirCommand( path )
	mkdirCommandString = "mkdir -p " .. path
	if IsHostOS("windows") then
		mkdirCommandString = "mkdir " .. path
		mkdirCommandString = string.gsub( mkdirCommandString, "/", "\\" )
		print( mkdirCommandString )
	end
	print( "mkdirCommand: " .. mkdirCommandString )
	return mkdirCommandString
end

function copyCommand( src, dest )
	copyCommandString = "cp " .. src .. " " .. dest 
	if IsHostOS("windows") then
		copyCommandString = "copy " .. src .. " " .. dest 
		copyCommandString = string.gsub( copyCommandString, "/", "\\" )
	end
	print( "copyComamndString: " .. copyCommandString )
	return copyCommandString
end

function SetupAndroidProject()
	local suffix = _OPTIONS["toolchain"]

	if _OPTIONS["toolchain"] == "android-gcc-arm" or 
	   _OPTIONS["toolchain"] == "android-gcc-x86" then
		kind "SharedLib"
		flags { "NoImportLib" }
		--kind "ConsoleApp"

		-- TODO: Other abis.
		local gdbserver = ANDROID_NDK_PATH .. "/prebuilt/android-arm/gdbserver/gdbserver"

		if _OPTIONS["toolchain"] == "android-gcc-arm" then
			abi = "armeabi-v7a"
		elseif _OPTIONS["toolchain"] == "android-gcc-x86" then
			abi = "x86"
		end

		local libName = solution().name
		buildPath = "../Build/" .. _ACTION .. "-" .. suffix

		projectPath = buildPath .. "/project"

		postBuildPath = "../" .. buildPath
		postBuildProjectPath = "../" .. projectPath

		-- Setup post build setp.
		configuration { "android-*" }
			PsyAddExternalLinks( "llvmcxxabi" )

			links {
				"GLESv1_CM",
				"GLESv2",
				"EGL",
				"OpenSLES",
				"llvmcxxabi"
			}

			androidTarget = "android-" .. GAME.android.sdk_version

		libPrefixName = "lib" .. libName
		libExt = ".so"

	end

end

function LLVMcxxabiProject()
	if _OPTIONS["toolchain"] == "android-gcc-arm" or 
	   _OPTIONS["toolchain"] == "android-gcc-x86" then
		project( "llvmcxxabi" )
			language( "C++" )
			configuration "*"
				kind ( EXTERNAL_PROJECT_KIND )
				buildoptions( "-std=c++11" )
				files { 
					"../Psybrus/External/libcxxabi/src/*",
					"../Psybrus/External/libcxxabi/include/*"
				}
				includedirs {
					"../Psybrus/External/libcxxabi/include"
				}
	end
end
