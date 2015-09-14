from Platform import *

vs2013Platform = Platform(
	"windows-vs-v120", "Windows (VS2013)", "windows",
	"x64", "windows", "vs2013", 
	[] )

vs2015Platform = Platform(
	"windows-vs-v140", "Windows (VS2015)", "windows",
	"x64", "windows", "vs2015", 
	[] )

vs2015dx12Platform = Platform(
	"windows-vs-v140", "Windows + DX12 (VS2015)", "windows",
	"x64", "windows", "vs2015", 
	[ "--with-dx12" ] )  # TODO: Have this as a subset of togglable options.


PLATFORMS.append( vs2013Platform )
PLATFORMS.append( vs2015Platform )
PLATFORMS.append( vs2015dx12Platform )
