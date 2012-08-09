-- Setup include paths for platform.
includedirs { "./Shared/**", "./Platforms/Windows/**" }

-- Shared.
dofile ("Source/Shared/premake4.lua")

-- Do platform specific stuff here.
dofile ("Source/Platforms/Windows/premake4.lua")

-- Integrations.
-- dofile ("Source/IntegrationGwen/premake4.lua")
dofile ("Source/IntegrationWwise/premake4.lua")
