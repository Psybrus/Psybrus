project "External_libnatpmp-20110808"
	kind "StaticLib"
	language "C"
	files { 
		"./getgateway.c",
		"./libnatpmpmodule.c",
		"./natpmp.c",
		"./natpmpc.c",
		"./wingettimeofday.c",
		"./*.h"
	 }

	includedirs { "./" }
