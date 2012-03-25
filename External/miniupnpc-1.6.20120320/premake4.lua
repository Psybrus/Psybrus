project "External_miniupnpc-1.6.20120320"
	kind "StaticLib"
	language "C"
	files { 
		"./connecthostport.c",
		"./igd_desc_parse.c",
		"./minisoap.c",
		"./miniupnpc.c",
		"./miniwget.c",
		"./minixml.c",
		"./portlistingparse.c",
		"./receivedata.c",
		"./upnpcommands.c",
		"./upnperrors.c",
		"./upnpreplyparse.c",
		"./*.h"
	 }

	includedirs { "./" }
