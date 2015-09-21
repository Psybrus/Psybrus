import json

def load():
	with open( "Game.json" ) as f:
		game_config_data = f.read()
		json_decode = json.JSONDecoder()
		GAME_CONFIG = json_decode.raw_decode( game_config_data )
		return GAME_CONFIG[0]
	return None
