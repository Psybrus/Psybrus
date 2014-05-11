-- Setup include paths for platform.
includedirs { "./Shared/", "./Platforms/Windows/" }

-- All the sub libraries.
dofile ("Source/Base_premake4.lua")
dofile ("Source/Engine_premake4.lua")
dofile ("Source/Events_premake4.lua")
dofile ("Source/Import_premake4.lua")
dofile ("Source/Math_premake4.lua")
dofile ("Source/Reflection_premake4.lua")
dofile ("Source/Serialisation_premake4.lua")
dofile ("Source/System_premake4.lua")
dofile ("Source/System_Content_premake4.lua")
dofile ("Source/System_Debug_premake4.lua")
dofile ("Source/System_File_premake4.lua")
dofile ("Source/System_Network_premake4.lua")
dofile ("Source/System_Os_premake4.lua")
dofile ("Source/System_Renderer_premake4.lua")
dofile ("Source/System_Scene_premake4.lua")
dofile ("Source/System_Sound_premake4.lua")
