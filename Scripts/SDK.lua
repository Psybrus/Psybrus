-- SDK.lua
-- Include paths,  

-- Setup boost stuff.
BOOST_INCLUDE_PATH = ""
BOOST_LIB_PATH = ""

if os.is("windows") then
	boostRoot = os.getenv("BOOST_ROOT") or "C:/Boost"
	BOOST_INCLUDE_PATH = boostRoot .. "/include/boost-1_56"
	BOOST_LIB_PATH = boostRoot .. "/lib"

else
	BOOST_INCLUDE_PATH = "/usr/include"
	BOOST_LIB_PATH = "/usr/lib"
end

-- Add boost libs to current project (all configurations)
function PsyAddBoostLibs( _links )
	configuration "vs*"
		includedirs{ BOOST_INCLUDE_PATH }
		libdirs{ BOOST_LIB_PATH }

	configuration "linux"
		links { _links }

end



-- Add system libraries
function PsyAddSystemLibs()
	configuration "windows"
      links {
         -- Windows libs.
         "user32",
         "gdi32",
         "opengl32",
         "winmm",
         "ws2_32",
         "IPHlpApi",
      }

    configuration "linux"
      links {
         -- Linux libs.
         "X11",
         "GL",
         "pthread",
         "SDL2"
      }

end
