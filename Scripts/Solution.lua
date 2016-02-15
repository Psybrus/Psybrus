-- Solution.lua
-- Common solution creation functions.

-- Setup a common solution.
function PsySolutionCommon( _name )
	solution( _name )

	print( "Adding Solution for " .. _ACTION .. " : " .. _name )

	-- All appropriate configurations we want.
	configurations { "Debug", "Release", "Production" }

	-- Setup toolchain.
	PsySetupToolchain()

	-- Terminate terminate solution.
	configuration "*"
end

-- Setup a game solution.
function PsySolutionGame( _name )
	PsySolutionCommon( _name )

	-- Terminate terminate solution.
	configuration "*"
end
