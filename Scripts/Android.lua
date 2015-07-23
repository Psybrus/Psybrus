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

	if _OPTIONS["toolchain"] == "android-clang-arm" or 
	   _OPTIONS["toolchain"] == "android-gcc-arm" then
		kind "SharedLib"
		flags { "NoImportLib" }

		-- TODO: Other abis.
		local gdbserver = ANDROID_NDK_PATH .. "/prebuilt/android-arm/gdbserver/gdbserver"

		local abi = "armeabi-v7a"
		local orientation = "landscape"

		local libName = solution().name
		buildPath = "../Build/" .. _ACTION .. "-" .. suffix

		projectPath = buildPath .. "/project"

		postBuildPath = "../" .. buildPath
		postBuildProjectPath = "../" .. projectPath

		packagePrefix = "com.psybrus."

		-- Create AndroidManifest.xml
		manifestFile = assert( io.open( projectPath .. "/AndroidManifest.xml", "w+" ) )
		manifestFile:write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" )
		manifestFile:write( "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n" )
		manifestFile:write( "          package=\"" .. packagePrefix .. libName .. "\"\n" )
		manifestFile:write( "          android:versionCode=\"1\"\n" )
		manifestFile:write( "          android:versionName=\"1.0\">\n" )
		manifestFile:write( "  <uses-sdk android:minSdkVersion=\"" .. ANDROID_SDK_VERSION .. "\" />\n" )
		manifestFile:write( "  <uses-permission android:name=\"android.permission.READ_EXTERNAL_STORAGE\" />\n" )
		manifestFile:write( "  <uses-feature android:glEsVersion=\"0x00020000\" />\n" )
		manifestFile:write( "  <application android:label=\"@string/app_name\"\n" )
		manifestFile:write( "               android:hasCode=\"false\" android:debuggable=\"true\">\n" )
		manifestFile:write( "    <activity android:name=\"android.app.NativeActivity\"\n" )
		manifestFile:write( "              android:screenOrientation=\"" .. orientation .. "\"\n" )
		manifestFile:write( "              android:label=\"@string/app_name\">\n" )
		manifestFile:write( "      <meta-data android:name=\"android.app.lib_name\"\n" )
		manifestFile:write( "                 android:value=\"" .. libName .. "\" />\n" )
		manifestFile:write( "      <intent-filter>\n" )
		manifestFile:write( "        <action android:name=\"android.intent.action.MAIN\" />\n" )
		manifestFile:write( "        <category android:name=\"android.intent.category.LAUNCHER\" />\n" )
		manifestFile:write( "      </intent-filter>\n" )
		manifestFile:write( "    </activity>\n" )
		manifestFile:write( "  </application>\n" )
		manifestFile:write( "</manifest>\n" )
		manifestFile:close()

		-- Create strings xml.
		stringsFile = assert( io.open( projectPath .. "/res/values/strings.xml", "w+" ) )
		stringsFile:write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" )
		stringsFile:write( "<resources>\n" )
		stringsFile:write( "  <string name=\"app_name\">" .. libName .. "</string>\n" )
		stringsFile:write( "</resources>\n" )
		stringsFile:close()

		-- Create gdb config.
		gdbFile = assert( io.open( projectPath .. "/libs/" .. abi .. "/gdb.setup", "w+" ) )
		gdbFile:write( "set solib-search-path ../obj\n" )
		gdbFile:write( "source " .. ANDROID_NDK_PATH .. "/prebuilt/common/gdb/common.setup\n" )
		gdbFile:write( "directory ../../../Source ../../../Psybrus/Engine/Source\n" )

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

			androidTarget = "android-" .. ANDROID_SDK_VERSION

		libPrefixName = "lib" .. libName .. "-gmake-" .. suffix
		libExt = ".so"

		os.execute( mkdirCommand( buildPath .. "/project" ) )
		os.execute( mkdirCommand( buildPath .. "/project/libs" ) )
		os.execute( mkdirCommand( buildPath .. "/project/libs/" .. abi ) )

		configuration { "android-*", "Debug" }
			postbuildcommands {
				--"$(SILENT) echo Copying packed content."
				--"$(SILENT) cp -r ../../Dist/PackedContent ./",
				"$(SILENT) echo Copying debug build...",
				"$(SILENT) " .. copyCommand( postBuildPath .. "/bin/" .. libPrefixName .. "-Debug" .. libExt, postBuildProjectPath .. "/libs/" .. abi .. "/lib" .. libName .. ".so" ),

				"$(SILENT) echo Copying gdbserver...",
				"$(SILENT) " .. copyCommand( gdbserver, postBuildProjectPath .. "/libs/" .. abi .. "/gdbserver" ),

				"$(SILENT) echo Updating project...",
				ANDROID_TOOL .. " update project -p " .. postBuildProjectPath .. " -t \"" .. androidTarget  .. "\" -n \"" .. libName ..  "\"",

				--"$(SILENT) cd " .. postBuildProjectPath,
				--"$(SILENT) ant debug",
				--"$(SILENT) adb install -r bin/" .. solution().name .. "-debug.apk"
			}

		configuration { "android-*", "Release" }
			postbuildcommands {
				--"$(SILENT) echo Copying packed content."
				--"$(SILENT) cp -r ../../Dist/PackedContent ./",
				"$(SILENT) echo Copying release build...",
				"$(SILENT) " .. copyCommand( postBuildPath .. "/bin/" .. libPrefixName .. "-Release" .. libExt, postBuildProjectPath .. "/libs/" .. abi .. "/lib" .. libName .. ".so" ),
				"$(SILENT) echo Updating project...",
				ANDROID_TOOL .. " update project -p " .. postBuildProjectPath .. " -t \"" .. androidTarget  .. "\" -n \"" .. libName ..  "\"",
				--"$(SILENT) cd " .. postBuildProjectPath,
				--"$(SILENT) ant debug",
				--"$(SILENT) adb install -r bin/" .. solution().name .. "-debug.apk"
			}

		configuration { "android-*", "Production" }
			postbuildcommands {
				--"$(SILENT) echo Copying packed content."
				--"$(SILENT) cp -r ../../Dist/PackedContent ./",
				"$(SILENT) echo Copying production build...",
				"$(SILENT) " .. copyCommand( postBuildPath .. "/bin/" .. libPrefixName .. "-Production" .. libExt, postBuildProjectPath .. "/libs/" .. abi .. "/lib" .. libName .. ".so" ),
				"$(SILENT) echo Updating project...",
				ANDROID_TOOL .. " update project -p " .. postBuildProjectPath .. " -t \"" .. androidTarget  .. "\" -n \"" .. libName ..  "\"",
				--"$(SILENT) cd " .. postBuildProjectPath,
				--"$(SILENT) ant debug",
				--"$(SILENT) adb install -r bin/" .. solution().name .. "-debug.apk"
			}
	end

end

function LLVMcxxabiProject()
	if _OPTIONS["toolchain"] == "android-clang-arm" or 
	   _OPTIONS["toolchain"] == "android-gcc-arm" then
		project( "llvmcxxabi" )
			language( "C++" )
			configuration "*"
				kind ( EXTERNAL_PROJECT_KIND )
				buildoptions( "-std=c++11" )
				files { 
					ANDROID_NDK_PATH .. "/sources/cxx-stl/llvm-libc++abi/libcxxabi/src/*",
					ANDROID_NDK_PATH .. "/sources/cxx-stl/llvm-libc++abi/libcxxabi/include/*"
				}
				includedirs {
					ANDROID_NDK_PATH .. "/sources/cxx-stl/llvm-libc++abi/libcxxabi/include"
				}
	end
end
