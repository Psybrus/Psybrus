project "External_stund"
	kind "StaticLib"
	language "C++"
	files { 
		"./stun.cxx",
		"./stun.h",
		"./udp.cxx",
		"./udp.h"
	 }

	includedirs { "./" }
