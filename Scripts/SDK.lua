-- SDK.lua
newoption {
	trigger = "boostpath",
	value = "path",
	description = "Boost path",
}

BOOST_INCLUDE_PATH = ""
BOOST_LIB_PATH = ""
BOOST_USE_SYSTEM = true

function SetupBoost()
	boostRoot = _OPTIONS["boostpath"]

	-- Setup boost if we have it
	if boostRoot then
		--
		BOOST_INCLUDE_PATH = _OPTIONS["boostpath"]
		BOOST_LIB_PATH = _OPTIONS["boostpath"] .. "/stage/lib"
		BOOST_USE_SYSTEM = false
	else
		if IsHostOS("windows") then
			boostRoot = os.getenv("BOOST_ROOT")
			BOOST_INCLUDE_PATH = boostRoot
			BOOST_LIB_PATH = boostRoot .. "/stage/lib"
			BOOST_USE_SYSTEM = false
		end
		if IsHostOS("macosx") then
			boostRoot = os.getenv("BOOST_ROOT")
			BOOST_INCLUDE_PATH = boostRoot
			BOOST_LIB_PATH = boostRoot .. "/stage/lib"
			BOOST_USE_SYSTEM = false
		end
	end

	if BOOST_USE_SYSTEM then
		BOOST_INCLUDE_PATH = "/usr/include" 
	end

	-- Load boost version.
	boostVersion = "NOT INSTALLED"
	local versionFileName = BOOST_INCLUDE_PATH .. "/boost/version.hpp"
	for versionLine in io.lines( versionFileName ) do
		version = string.match( versionLine, "%d_%d%d" )
		if version then
			boostVersion = version
		end
	end

	-- Check version is one which we support and know to work.
	validVersions = {
		[ "1_54" ] = true,
		[ "1_55" ] = true,
		[ "1_56" ] = true,
		[ "1_57" ] = true,
		[ "1_58" ] = true,
		[ "1_59" ] = true
	}

	if not validVersions[ boostVersion ] then
		print ( "Invalid boost version specified. (path to " .. boostVersion .. " given, " .. BOOST_INCLUDE_PATH .. ")" )
		os.exit(1)		
	end

	print( "Using Boost version " .. boostVersion )

	if BOOST_USE_SYSTEM then
		BOOST_INCLUDE_PATH = ""
	end
end

-- Add boost libs to current project (all configurations)
function PsyAddBoostLibs( _links )
	configuration "windows-*"
		includedirs { BOOST_INCLUDE_PATH }
		libdirs { BOOST_LIB_PATH }

	configuration "linux-*"
		-- If we aren't using system, setup include + linkage.
		if not BOOST_USE_SYSTEM then
			includedirs { BOOST_INCLUDE_PATH }
			libdirs { BOOST_LIB_PATH }
			linkoptions { "-Wl,-rpath-link," .. BOOST_LIB_PATH }
		end

		for i, link in ipairs( _links ) do
			links { "boost_" .. link }
		end

	configuration "osx-*"
		-- If we aren't using system, setup include + linkage.
		if not BOOST_USE_SYSTEM then
			includedirs { BOOST_INCLUDE_PATH }
			libdirs { BOOST_LIB_PATH }
		end

		for i, link in ipairs( _links ) do
			links { "boost_" .. link }
		end

	configuration "*"
end


-- Add system libraries
function PsyAddSystemLibs()
if _OPTIONS["with-vk"] then
      VK_SDK_PATH = os.getenv("VK_SDK_PATH")
      libdirs {
          VK_SDK_PATH .. "/Source/lib"
      }

      links {
          "vulkan.0"
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
end

SetupBoost()
