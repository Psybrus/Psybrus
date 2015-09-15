from Platform import *
from BuildGmake import *

osxPlatform = Platform(
	"osx-clang", "OSX (Clang)", "osx",
	"x64", "macosx", "gmake", BuildGmake,
	[] )

PLATFORMS.append( osxPlatform )

