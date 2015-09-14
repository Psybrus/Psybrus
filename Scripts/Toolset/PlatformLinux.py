from Platform import *

linuxPlatform = Platform(
	"linux-clang", "Linux (Clang 3.5)", "linux",
	"x64", "linux", "gmake", 
	[] )

PLATFORMS.append( linuxPlatform )

