from Platform import *

osxPlatform = Platform(
	"osx-clang", "OSX (Clang)", "osx",
	"x64", "macosx", "gmake", 
	[] )

PLATFORMS.append( osxPlatform )

