from Platform import *
from BuildGmake import *
from Deploy import *

linuxPlatform = Platform(
	"linux-clang", "Linux (Clang 3.5)", "linux",
	"x64", "linux", "gmake", BuildGmake, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

PLATFORMS.append( linuxPlatform )

