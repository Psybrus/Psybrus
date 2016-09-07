from Platform import *
from BuildGmake import *
from DeployAndroid import *

android_plaform_arm_clang = Platform( 
	"android-clang-arm", "Android ARM (Clang)", "android",
	"x32", "linux", "gmake", BuildGmake, DeployAndroid,
	"Psybrus/Dist/Platforms/android.json",
	[] )

PLATFORMS.append( android_plaform_arm_clang )

android_plaform_x86_clang = Platform( 
	"android-clang-x86", "Android x86 (Clang)", "android",
	"x32", "linux", "gmake", BuildGmake, DeployAndroid,
	"Psybrus/Dist/Platforms/android.json",
	[] )

PLATFORMS.append( android_plaform_x86_clang )
