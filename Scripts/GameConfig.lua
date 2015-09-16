-- Load in game config.
JSON = (loadfile "JSON.lua")()
local gameFile = assert( io.open( "../../Game.json", "r" ) )
local gameFileContent = gameFile:read("*all")
GAME = JSON:decode( gameFileContent )
