if PsyProjectExternalDLL( "SDL", "C" ) then
	configuration "*"
		kind ( EXTERNAL_DLL_PROJECT_KIND )
		includedirs {
			"SDL-mirror/include",
			"SDL-mirror/src",
		}
		files {
			"SDL-mirror/src/*.c",
			"SDL-mirror/src/*.h",
			"SDL-mirror/src/atomic/*.c",
			"SDL-mirror/src/atomic/*.h",
			"SDL-mirror/src/audio/**.c",
			"SDL-mirror/src/audio/**.h",
			"SDL-mirror/src/cpuinfo/*.c",
			"SDL-mirror/src/cpuinfo/*.h",
			"SDL-mirror/src/dynapi/*.c",
			"SDL-mirror/src/dynapi/*.h",
			"SDL-mirror/src/events/*.c",
			"SDL-mirror/src/events/*.h",
			"SDL-mirror/src/file/*.c",
			"SDL-mirror/src/file/*.h",
			"SDL-mirror/src/haptic/*.c",
			"SDL-mirror/src/haptic/*.h",
			"SDL-mirror/src/joystick/*.c",
			"SDL-mirror/src/joystick/*.h",
			"SDL-mirror/src/libm/*.c",
			"SDL-mirror/src/libm/*.h",
			"SDL-mirror/src/power/*.c",
			"SDL-mirror/src/power/*.h",
			"SDL-mirror/src/render/*.c",
			"SDL-mirror/src/render/*.h",
			"SDL-mirror/src/render/direct3d/*.c",
			"SDL-mirror/src/render/direct3d/*.h",
			"SDL-mirror/src/render/direct3d11/*.c",
			"SDL-mirror/src/render/direct3d11/*.h",
			"SDL-mirror/src/render/opengl/*.c",
			"SDL-mirror/src/render/opengl/*.h",
			"SDL-mirror/src/render/opengles2/*.c",
			"SDL-mirror/src/render/opengles2/*.h",
			"SDL-mirror/src/render/software/*.c",
			"SDL-mirror/src/render/software/*.h",
			"SDL-mirror/src/stdlib/*.c",
			"SDL-mirror/src/stdlib/*.h",
			"SDL-mirror/src/thread/*.c",
			"SDL-mirror/src/thread/*.h",
			"SDL-mirror/src/timer/*.c",
			"SDL-mirror/src/timer/*.h",
			"SDL-mirror/src/video/*.c",
			"SDL-mirror/src/video/*.h",
			"SDL-mirror/src/video/dummy/*.c",
			"SDL-mirror/src/video/dummy/*.h",
		}

	configuration "windows-*"
		files {
			"SDL-mirror/src/core/windows/*.c",
			"SDL-mirror/src/core/windows/*.h",
			"SDL-mirror/src/filesystem/windows/*.c",
			"SDL-mirror/src/filesystem/windows/*.h",
			"SDL-mirror/src/haptic/windows/*.c",
			"SDL-mirror/src/haptic/windows/*.h",
			"SDL-mirror/src/joystick/windows/*.c",
			"SDL-mirror/src/joystick/windows/*.h",
			"SDL-mirror/src/loadso/windows/*.c",
			"SDL-mirror/src/loadso/windows/*.h",
			"SDL-mirror/src/power/windows/*.c",
			"SDL-mirror/src/power/windows/*.h",
			"SDL-mirror/src/thread/generic/SDL_syscond.c",
			"SDL-mirror/src/thread/windows/*.c",
			"SDL-mirror/src/thread/windows/*.h",
			"SDL-mirror/src/timer/windows/*.c",
			"SDL-mirror/src/timer/windows/*.h",
			"SDL-mirror/src/video/windows/*.c",
			"SDL-mirror/src/video/windows/*.h",
		}

		links {
			"user32",
			"gdi32",
			"winmm",
			"ws2_32",
			"IPHlpApi",
			"xinput",
			"ole32",
			"oleaut32",
			"imm32",
			"version",
		}

	configuration "linux-*"
		files {
			"SDL-mirror/src/core/linux/*.c",
			"SDL-mirror/src/core/linux/*.h",
			"SDL-mirror/src/filesystem/unix/*.c",
			"SDL-mirror/src/filesystem/unix/*.h",
			"SDL-mirror/src/haptic/windows/*.c",
			"SDL-mirror/src/haptic/windows/*.h",
			"SDL-mirror/src/joystick/linux/*.c",
			"SDL-mirror/src/joystick/linux/*.h",
			"SDL-mirror/src/loadso/dlopen/*.c",
			"SDL-mirror/src/loadso/dlopen/*.h",
			"SDL-mirror/src/power/linux/*.c",
			"SDL-mirror/src/power/linux/*.h",
			"SDL-mirror/src/thread/pthread/*.c",
			"SDL-mirror/src/thread/pthread/*.h",
			"SDL-mirror/src/timer/unix/*.c",
			"SDL-mirror/src/timer/unix/*.h",
			"SDL-mirror/src/video/wayland/*.c",
			"SDL-mirror/src/video/wayland/*.h",
			"SDL-mirror/src/video/x11/*.c",
			"SDL-mirror/src/video/x11/*.h",
		}

		links {
			"X11",
			"GL",
			"pthread",
			"bluetooth",
			"dl"
		}

	configuration "osx-*"
		files {
			"SDL-mirror/src/audio/coreaudio/*.c",
			"SDL-mirror/src/audio/coreaudio/*.h",
			"SDL-mirror/src/audio/disk/*.c",
			"SDL-mirror/src/audio/disk/*.c",
			"SDL-mirror/src/audio/dummy/*.h",
			"SDL-mirror/src/audio/dummy/*.h",
			"SDL-mirror/src/file/cocoa/*.m",
			"SDL-mirror/src/file/cocoa/*.h",
			"SDL-mirror/src/filesystem/cocoa/*.m",
			"SDL-mirror/src/filesystem/cocoa/*.h",
			"SDL-mirror/src/filesystem/unix/*.c",
			"SDL-mirror/src/filesystem/unix/*.h",
			"SDL-mirror/src/haptic/darwin/*.c",
			"SDL-mirror/src/haptic/darwin/*.h",
			"SDL-mirror/src/joystick/darwin/*.c",
			"SDL-mirror/src/joystick/darwin/*.h",
			"SDL-mirror/src/loadso/dlopen/*.c",
			"SDL-mirror/src/loadso/dlopen/*.h",
			"SDL-mirror/src/power/macosx/*.c",
			"SDL-mirror/src/power/macosx/*.h",
			"SDL-mirror/src/power/uikit/*.m",
			"SDL-mirror/src/power/uikit/*.h",
			"SDL-mirror/src/thread/pthread/*.c",
			"SDL-mirror/src/thread/pthread/*.h",
			"SDL-mirror/src/timer/unix/*.c",
			"SDL-mirror/src/timer/unix/*.h",
			"SDL-mirror/src/video/cocoa/*.m",
			"SDL-mirror/src/video/cocoa/*.h",
		}

		links {
			"AppKit.framework",
			"AudioUnit.framework",
			"Carbon.framework",
			"Cocoa.framework",
			"CoreAudio.framework",
			"CoreFoundation.framework",
			"CoreVideo.framework",
			"ForceFeedback.framework",
			"IOKit.framework",
			"OpenGL.framework",
			"pthread",
			"dl"
		}

	configuration "windows-* or linux-* or osx-*"
		targetdir ( "../../Dist" )

	configuration "html5-*"
		kind ( EXTERNAL_PROJECT_KIND )
		defines { 
			"SDL_AUDIO_DRIVER_EMSCRIPTEN=1",
			"SDL_JOYSTICK_EMSCRIPTEN=1",
			"SDL_VIDEO_DRIVER_EMSCRIPTEN=1",
			"SDL_VIDEO_OPENGL_EGL=1",
			"SDL_VIDEO_OPENGL_ES2=1",
		}
		files {
			"SDL-mirror/src/audio/emscripten/*.c",
			"SDL-mirror/src/audio/emscripten/*.h",
			"SDL-mirror/src/filesystem/emscripten/*.c",
			"SDL-mirror/src/filesystem/emscripten/*.h",
			"SDL-mirror/src/haptic/dummy/*.c",
			"SDL-mirror/src/haptic/dummy/*.h",
			"SDL-mirror/src/joystick/emscripten/*.c",
			"SDL-mirror/src/joystick/emscripten/*.h",
			"SDL-mirror/src/loadso/dummy/*.c",
			"SDL-mirror/src/loadso/dummy/*.h",
			"SDL-mirror/src/power/emscripten/*.c",
			"SDL-mirror/src/power/emscripten/*.h",
			"SDL-mirror/src/thread/generic/*.c",
			"SDL-mirror/src/thread/generic/*.h",
			"SDL-mirror/src/timer/dummy/*.c",
			"SDL-mirror/src/timer/dummy/*.h",
			"SDL-mirror/src/video/emscripten/*.c",
			"SDL-mirror/src/video/emscripten/*.h",
		}
end
