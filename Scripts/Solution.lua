-- Solution.lua
-- Common solution creation functions.

-- Setup a common solution.
function PsySolutionCommon( _name )
	solution( _name )

	print( "Adding Solution for " .. _ACTION .. " : " .. _name )

	PsySetupToolchain()

	-- Solution location.
	location ( "Build/" .. _ACTION .. "-" .. _OPTIONS[ "toolchain" ] )

	-- All appropriate configurations we want.
	configurations { "Debug", "Release", "Profile", "Production" }

	-- Target dirs.
	configuration "Debug"
		targetdir ( "Build/" .. _ACTION .. "/bin/Debug" )

	configuration "Release"
		targetdir ( "Build/" .. _ACTION .. "/bin/Release" )

	configuration "Profile"
		targetdir ( "Build/" .. _ACTION .. "/bin/Profile" )

	configuration "Production"
		targetdir ( "Build/" .. _ACTION .. "/bin/Production" )

	-- Terminate terminate solution.
	configuration "*"
end

-- Setup a game solution.
function PsySolutionGame( _name )
	PsySolutionCommon( _name )

	-- Terminate terminate solution.
	configuration "*"
end
