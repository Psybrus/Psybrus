from Platform import *
from BuildGmake import *
from DeployAndroid import *

androidPlaform = Platform( 
	"android-gcc-arm", "Android ARM (GCC 4.9)", "android",
	"x32", "linux", "gmake", BuildGmake, DeployAndroid,
	"Psybrus/Dist/Platforms/android.json",
	[] )

PLATFORMS.append( androidPlaform )

