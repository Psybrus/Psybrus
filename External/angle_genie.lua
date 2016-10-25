if PsyProjectExternalDLL( "angle", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_DLL_PROJECT_KIND )
		targetdir ( "../../Dist" )
		defines {
			"ANGLE_ENABLE_ESSL=1",
			"ANGLE_ENABLE_GLSL=1",
			"LIBANGLE_IMPLEMENTATION=1",
			"GL_GLEXT_PROTOTYPES=1",
			"GL_APICALL=__declspec(dllexport)",
			"GL_APIENTRY=__stdcall",
			"EGLAPI=__declspec(dllexport)",
			"EGLAPIENTRY=__stdcall",
		}
		includedirs {
			"autogen/angle",
			"angle/include",
			"angle/src",
			"angle/src/common/third_party/numerics",
		}
		files {
			"angle/include/**.h",
			"angle/src/common/**.cpp",
			"angle/src/common/**.h",
			"angle/src/compiler/**.cpp",
			"angle/src/compiler/**.h",
			"angle/src/image_util/**.cpp",
			"angle/src/image_util/**.h",
			"angle/src/libANGLE/*.cpp",
			"angle/src/libANGLE/*.h",
			"angle/src/libANGLE/renderer/*.cpp",
			"angle/src/libANGLE/renderer/*.h",
			"angle/src/libEGL/*.cpp",
			"angle/src/libEGL/*.h",
			"angle/src/libGLESv2/*.cpp",
			"angle/src/libGLESv2/*.h",
			"angle/src/third_party/compiler/*.cpp",
			"angle/src/third_party/compiler/*.h",
			"angle/src/third_party/murmurhash/*.cpp",
			"angle/src/third_party/murmurhash/*.h",
		}

		excludes {
			"angle/src/common/*_unittest.cpp",
			"angle/src/libANGLE/*_unittest.cpp",
			"angle/src/libEGL/*_unittest.cpp",
			"angle/src/libGLESv2/*_unittest.cpp",

			-- Exclude these as we don't want
			-- normal GL entrypoints.
			--"angle/src/libEGL/libEGL.cpp",
			--"angle/src/libGLESv2/libGLESv2.cpp",
		}

	configuration "windows-*"
		defines {
			"ANGLE_ENABLE_D3D11=1",
			"ANGLE_ENABLE_HLSL=1",
			"NOMINMAX=1"
		}

		files {
			"angle/src/libANGLE/renderer/d3d/*.cpp",
			"angle/src/libANGLE/renderer/d3d/*.h",
			"angle/src/libANGLE/renderer/d3d/d3d11/*.cpp",
			"angle/src/libANGLE/renderer/d3d/d3d11/*.h",
			"angle/src/libANGLE/renderer/d3d/d3d11/win32/*.cpp",
			"angle/src/libANGLE/renderer/d3d/d3d11/win32/*.h",
			"angle/src/libANGLE/renderer/d3d/d3d11/shaders/*.hlsl",
			"angle/src/libANGLE/renderer/d3d/d3d11/shaders/**.h",
		}

		links {
			"dxguid"
		}
end
