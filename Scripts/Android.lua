dofile( "NDK/ndk.lua" )
dofile( "NDK/common.lua" )
dofile( "NDK/makefile.lua" )
dofile( "NDK/manifest.lua" )

function SetupAndroidProject()
	local suffix = _OPTIONS["toolchain"]

	if _OPTIONS["toolchain"] == "android-clang-arm" or 
	   _OPTIONS["toolchain"] == "android-clang-x86" then
		kind "SharedLib"
		flags { "NoImportLib" }
		--kind "ConsoleApp"

		configuration { "android-*" }
			PsyAddExternalLinks( "llvmcxxabi" )

			links {
				"c",
				"gcc",
				"dl",
				"log",
				"GLESv1_CM",
				"GLESv2",
				"EGL",
				"OpenSLES",
				"llvmcxxabi",
				"llvmcxxabi_unwind",
			}
	end

end

function LLVMcxxabiProject()
	if _OPTIONS["toolchain"] == "android-clang-arm" or 
	   _OPTIONS["toolchain"] == "android-clang-x86" then
		project( "llvmcxxabi" )
			language( "C++" )
			configuration "*"
				kind ( EXTERNAL_PROJECT_KIND )
				buildoptions( "-std=c++11" )
				defines { "LIBCXXABI_USE_LLVM_UNWINDER=1" }
				files { 
					"../Psybrus/External/libcxxabi/src/abort_message.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_aux_runtime.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_default_handlers.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_demangle.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_exception.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_exception_storage.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_guard.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_handlers.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_new_delete.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_personality.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_thread_atexit.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_unexpected.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_vector.cpp",
					"../Psybrus/External/libcxxabi/src/cxa_virtual.cpp",
					"../Psybrus/External/libcxxabi/src/exception.cpp",
					"../Psybrus/External/libcxxabi/src/private_typeinfo.cpp",
					"../Psybrus/External/libcxxabi/src/stdexcept.cpp",
					"../Psybrus/External/libcxxabi/src/typeinfo.cpp",
					"../Psybrus/External/libcxxabi/include/*"
				}
				includedirs {
					"../Psybrus/External/libcxxabi/include"
				}

		project( "llvmcxxabi_unwind" )
			language( "C++" )
			configuration "*"
				kind ( EXTERNAL_PROJECT_KIND )
				files { 
					"../Psybrus/External/libcxxabi/src/Unwind/libunwind.cpp",
					"../Psybrus/External/libcxxabi/src/Unwind/Unwind-EHABI.cpp",
					"../Psybrus/External/libcxxabi/src/Unwind/Unwind-sjlj.c",
					"../Psybrus/External/libcxxabi/src/Unwind/UnwindLevel1.c",
					"../Psybrus/External/libcxxabi/src/Unwind/UnwindLevel1-gcc-ext.c",
					"../Psybrus/External/libcxxabi/src/Unwind/UnwindRegistersRestore.S",
					"../Psybrus/External/libcxxabi/src/Unwind/UnwindRegistersSave.S",
				}
				includedirs {
					"../Psybrus/External/libcxxabi/include"
				}
	end
end
