-- SDK.lua

-- Add system libraries
function PsyAddSystemLibs()
if _OPTIONS["with-vk"] then
      VK_SDK_PATH = os.getenv("VK_SDK_PATH")
      libdirs {
          VK_SDK_PATH .. "/Source/lib"
      }

      links {
          "vulkan-1"
      }
end

	configuration "windows-*"
		links {
			"user32",
			"gdi32",
			"opengl32",
			"winmm",
			"ws2_32",
			"IPHlpApi",
			"xinput"
		}

	configuration "winphone-*"
		links {
			"WindowsPhoneCore",
			"RuntimeObject",
			"PhoneAppModelHost",
			"ws2_32"
		}

	 configuration "linux-*"
		links {
			"X11",
			"GL",
			"pthread",
			"bluetooth",
			"SDL2",
			"dl"
		}

	 configuration "osx-*"
		links {
			"AppKit.framework",
			"CoreFoundation.framework",
			"OpenGL.framework",
			"SDL2",
			"pthread",
			"dl"
		}
		libdirs {
			"/usr/local/Cellar/sdl2/2.0.3/lib"
		}
end
