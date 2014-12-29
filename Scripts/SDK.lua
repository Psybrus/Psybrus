-- SDK.lua
newoption {
	trigger = "boostpath",
	value = "path",
	description = "Boost path",
}

BOOST_INCLUDE_PATH = ""
BOOST_LIB_PATH = ""
BOOST_USE_SYSTEM = false

function SetupBoost()
	-- Setup boost if we have it
	if _OPTIONS["boostpath"] then
		--
		BOOST_INCLUDE_PATH = _OPTIONS["boostpath"]
		BOOST_LIB_PATH = _OPTIONS["boostpath"] .. "/stage/lib"
	else
		if os.is("windows") then
			boostRoot = os.getenv("BOOST_ROOT") or "C:/Boost"
			BOOST_INCLUDE_PATH = boostRoot .. "/include/boost-1_56"
			BOOST_LIB_PATH = boostRoot .. "/lib"
		else
			BOOST_USE_SYSTEM = true
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
	configuration "vs*"
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
end


-- Add system libraries
function PsyAddSystemLibs()
	configuration "windows-*"
		links {
			-- Windows libs.
			"user32",
			"gdi32",
			"opengl32",
			"winmm",
			"ws2_32",
			"IPHlpApi",
		}

	 configuration "linux-*"
		links {
			-- Linux libs.
			"X11",
			"GL",
			"pthread",
			"SDL2"
		}

end

SetupBoost()
