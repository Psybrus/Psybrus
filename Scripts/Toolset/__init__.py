import subprocess
import os
from platform import system

# Import platform library straight into toolset.
# TODO: Tidy up later.
from Platform import *

# Import our platforms depending on host platform.
if system() == "Darwin":
	import PlatformAndroid
	import PlatformHTML5
	import PlatformOSX
elif system() == "Linux":
	import PlatformAndroid
	import PlatformHTML5
	import PlatformLinux
elif system() == "Windows":
	import PlatformAndroid
	import PlatformWindows
	import PlatformWinPhone
	
def getToolsets():
	ret = []
	for platform in PLATFORMS:
		ret.append(platform.name)
	return ret

def getConfigs():
	return [ "debug", "release", "profile", "production" ]

def findPlatform( _name ):
	for platform in PLATFORMS:
		if platform.name == _name:
			return platform
	return None
