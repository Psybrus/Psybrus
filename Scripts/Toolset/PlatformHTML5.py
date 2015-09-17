from Platform import *
from BuildGmake import *
from Deploy import *

html5Platform = Platform( 
	"html5-clang-asmjs", "HTML5 asm.js (Clang)", "html5",
	"x32", "linux", "gmake", BuildGmake, Deploy,
	"Psybrus/Dist/Platforms/html5.json",
	[] )

PLATFORMS.append( html5Platform )

