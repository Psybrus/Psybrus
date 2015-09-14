import subprocess
import os
import platform

# Import platform library straight into toolset.
# TODO: Tidy up later.
from Platform import *

# Import our platforms depending on host platform.
if platform.system() == "Darwin":
	import PlatformAndroid
	import PlatformHTML5
	import PlatformOSX
elif platform.system() == "Linux":
	import PlatformAndroid
	import PlatformHTML5
	import PlatformLinux
elif platform.system() == "Windows":
	import PlatformAndroid
	import PlatformWindows


class ToolGmake():
	def __init__( self, _tool, _path ):
		self.tool = _tool
		self.path = _path
		
		# TODO: Should make generic to replace all env vars found.
		env = os.environ
		self.tool = self.tool.replace( "%ANDROID_NDK%", env["ANDROID_NDK"] )

TOOLS_GMAKE = {
	"Windows" : ToolGmake(
		"%ANDROID_NDK%\\prebuilt\\windows-x86_64\\bin\\make",
		"C:\\Windows\\System32" ),

	"Linux" : ToolGmake(
		"make", 
		None ),

	"Darwin" : ToolGmake(
		"make",
		None ),
}

MSBUILD = "C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe"

def getToolsets():
	ret = []
	for platform_info in PLATFORMS:
		ret.append(platform_info.name)
	return ret

def getConfigs():
	return [ "debug", "release", "profile", "production" ]

def findPlatformInfo( _name ):
	for platform_info in PLATFORMS:
		if platform_info.name == _name:
			return platform_info
	return None


def buildToolset( toolset, config ):
	# Force to lowercase.
	toolset = toolset.lower()
	config = config.lower()

	# Check validity of toolset.
	if not (toolset in getToolsets()):
		raise Exception( "Must select valid toolset." )

	# Check validity of config.
	if not (config in getConfigs()):
		raise Exception( "Must select validconfig." )

	# Build.
	platform_info = findPlatformInfo( toolset )
	if platform_info.project_type == "gmake":
		build_gmake( platform_info, config )
	if platform_info.project_type == "vs2013":
		build_vs2013( platform_info, config )
	if platform_info.project_type == "vs2015":
		build_vs2015( platform_info, config )

def build_gmake( _platform_info, config ):
	gmake_tool = TOOLS_GMAKE[ platform.system() ];

	gmake_executable = gmake_tool.tool
	gmake_path_env = gmake_tool.path

	env = os.environ
	if gmake_path_env != None:
		env[ "PATH" ] = gmake_path_env

	gmake_command = gmake_executable + " -j5 config=" + config
	gmake_path = os.path.join( "Projects", _platform_info.project_type + "-" + _platform_info.name )

	print "Launching: " + gmake_command

	cwd = os.getcwd()
	try:
		os.chdir( gmake_path )
		subprocess.Popen( gmake_command, env=env, shell=True ).communicate()
	except KeyboardInterrupt:
		print "Build cancelled. Exiting."
		exit(1)
	finally:
		os.chdir( cwd )

def build_vs( _platform_info, config ):
	build_command = MSBUILD + " "
	build_path = os.path.join( "Projects", _platform_info.project_type + "-" + _platform_info.name )
	env = os.environ
	print "Launching: " + build_command
	cwd = os.getcwd()
	try:
		os.chdir( build_path )
		subprocess.Popen( build_command, env=env, shell=True ).communicate()
	except KeyboardInterrupt:
		print "Build cancelled. Exiting."
		exit(1)
	finally:
		os.chdir( cwd )

	MSBUILD