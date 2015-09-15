from Platform import *
from BuildGmake import *

html5Platform = Platform( 
	"asmjs", "HTML5 (Clang)", "html5",
	"x32", "linux", "gmake", BuildGmake,
	[] )

PLATFORMS.append( html5Platform )

