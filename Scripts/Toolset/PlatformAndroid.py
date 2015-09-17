from Platform import *
from BuildGmake import *
from DeployAndroid import *

android_plaform_arm = Platform( 
	"android-gcc-arm", "Android ARM (GCC 4.9)", "android",
	"x32", "linux", "gmake", BuildGmake, DeployAndroid,
	"Psybrus/Dist/Platforms/android.json",
	[] )

PLATFORMS.append( android_plaform_arm )

android_plaform_x86 = Platform( 
	"android-gcc-x86", "Android x86 (GCC 4.9)", "android",
	"x32", "linux", "gmake", BuildGmake, DeployAndroid,
	"Psybrus/Dist/Platforms/android.json",
	[] )

PLATFORMS.append( android_plaform_x86 )
