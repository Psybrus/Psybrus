from Platform import *
from Build import *
from BuildMSBuild import *
from Deploy import *

vs2015Platform = Platform(
	"windows-vs-v140", "Windows (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[] )

vs2015anglePlatform = Platform(
	"windows-vs-v140", "Windows + ANGLE (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[ "--with-angle" ] )  # TODO: Have this as a subset of togglable options.

vs2015dx12Platform = Platform(
	"windows-vs-v140", "Windows + DX12 (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[ "--with-dx12" ] )  # TODO: Have this as a subset of togglable options.

vs2015dx12vkPlatform = Platform(
	"windows-vs-v140", "Windows + DX12 + Vulkan (VS2015)", "windows",
	"x64", "windows", "vs2015", BuildMSBuild, Deploy, 
	"Psybrus/Dist/Platforms/pc.json",
	[ "--with-dx12", "--with-vk" ] )  # TODO: Have this as a subset of togglable options.


PLATFORMS.append( vs2015Platform )
PLATFORMS.append( vs2015anglePlatform )
PLATFORMS.append( vs2015dx12Platform )
PLATFORMS.append( vs2015dx12vkPlatform )
