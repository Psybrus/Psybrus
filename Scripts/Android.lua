dofile( "NDK/ndk.lua" )
dofile( "NDK/common.lua" )
dofile( "NDK/makefile.lua" )
dofile( "NDK/manifest.lua" )

function SetupAndroidProject()
	local suffix = _OPTIONS["toolchain"]

	if _OPTIONS["toolchain"] == "android-gcc-arm" or 
	   _OPTIONS["toolchain"] == "android-gcc-x86" then
		kind "SharedLib"
		flags { "NoImportLib" }
		--kind "ConsoleApp"

		configuration { "android-*" }
			PsyAddExternalLinks( "llvmcxxabi" )

			links {
				"GLESv1_CM",
				"GLESv2",
				"EGL",
				"OpenSLES",
				"llvmcxxabi"
			}
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
