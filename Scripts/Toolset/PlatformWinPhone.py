from Platform import *
from Build import *
from BuildMSBuild import *
from Deploy import *

vs2013wp81Platform = Platform(
	"winphone-vs-v120_wp81", "Windows Phone (8.1)", "windows",
	"ARM", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

PLATFORMS.append( vs2013wp81Platform )
