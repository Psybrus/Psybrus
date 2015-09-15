from Platform import *
from BuildGmake import *

linuxPlatform = Platform(
	"linux-clang", "Linux (Clang 3.5)", "linux",
	"x64", "linux", "gmake", BuildGmake,
	[] )

PLATFORMS.append( linuxPlatform )

