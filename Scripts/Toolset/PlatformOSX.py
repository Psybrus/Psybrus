from Platform import *
from BuildGmake import *
from Deploy import *

osxPlatform = Platform(
	"osx-clang", "OSX (Clang)", "osx",
	"x64", "macosx", "gmake", BuildGmake, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

PLATFORMS.append( osxPlatform )

