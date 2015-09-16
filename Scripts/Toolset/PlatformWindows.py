from Platform import *
from Build import *
from BuildMSBuild import *
from Deploy import *

vs2013Platform = Platform(
	"windows-vs-v120", "Windows (VS2013)", "windows",
	"x64", "windows", "vs2013", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

vs2015Platform = Platform(
	"windows-vs-v140", "Windows (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

vs2015dx12Platform = Platform(
	"windows-vs-v140", "Windows + DX12 (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[ "--with-dx12" ] )  # TODO: Have this as a subset of togglable options.


PLATFORMS.append( vs2013Platform )
PLATFORMS.append( vs2015Platform )
PLATFORMS.append( vs2015dx12Platform )
