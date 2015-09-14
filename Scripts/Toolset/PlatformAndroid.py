from Platform import *

androidPlaform = Platform( 
	"android-gcc-arm", "Android ARM (GCC 4.9)", "android",
	"x32", "linux", "gmake",
	[] )

PLATFORMS.append( androidPlaform )

