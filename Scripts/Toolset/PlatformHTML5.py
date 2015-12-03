from Platform import *
from BuildGmake import *
from DeployHTML5 import *

html5Platform = Platform( 
	"html5-clang-asmjs", "HTML5 asm.js (Clang)", "html5",
	"x32", "linux", "gmake", BuildGmake, DeployHTML5,
	"Psybrus/Dist/Platforms/html5.json",
	[] )

PLATFORMS.append( html5Platform )

