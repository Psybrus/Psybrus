-- Setup a common solution.
function PsySolutionCommon( _name )
	solution( _name )

	-- Solution location.
	location ( "Build/" .. _ACTION )

	-- All appropriate configurations we want.
	configurations { "Debug", "Release", "Profile", "Production" }

	-- Target dirs.
	configuration "Debug"
		targetdir ( "Build/" .. _ACTION .. "bin/Debug" )

	configuration "Release"
		targetdir ( "Build/" .. _ACTION .. "bin/Release" )

	configuration "Profile"
		targetdir ( "Build/" .. _ACTION .. "bin/Profile" )

	configuration "Production"
		targetdir ( "Build/" .. _ACTION .. "bin/Production" )

end

-- Setup a game solution.
function PsySolutionGame( _name )
	PsySolutionCommon( _name )


end
