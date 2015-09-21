import copy
import glob
import re
import os
import shutil
import subprocess
from platform import system

from Deploy import *

import GameConfig
import Log

def createDir( dir ):
	try:
		os.mkdir( dir )
	except:
		pass

class DeployAndroid( Deploy ):
	def __init__( self, _platform ):
		Deploy.__init__( self, _platform )

		self.android_project_root = os.path.join( "Build", _platform.project_type + "-" + _platform.name, "project" )

		# Load in config.
		self.game_config = GameConfig.load()
		self.game_name = self.game_config[ "name" ]
		self.package = self.game_config[ "android" ][ "package" ]
		self.sdk_version = self.game_config[ "android" ][ "sdk_version" ]
		self.ndk_version = self.game_config[ "android" ][ "ndk_version" ]

	def package_build( self, _config ):
		Deploy.package_build( self, _config )

		# Create project directory.
		Log.write( "Creating project directory..." )
		createDir( self.android_project_root )
		createDir( os.path.join( self.android_project_root, "libs" ) )

		# Update project.
		target = "android-" + self.sdk_version
		self.launch_android_tool( "update project -p ./ -t \"" + target + "\" -n \"" + self.game_name + "\"" )

		# Copy build(s) in to target directory.
		configs = {
			"debug" : "Debug",
			"release" : "Release",
			"profile" : "Profile",
			"production" : "Production",
		}

		abis = {
			"android-gcc-arm" : "armeabi-v7a",
			"android-gcc-x86" : "x86"
		}
		abi = abis[ self.platform.name ]

		srcBuilds = [
			os.path.join( self.android_project_root, "..", "bin", "lib" + self.game_name + "-" + configs[ _config ] + ".so" )
		]

		destBuilds = [
			os.path.join( self.android_project_root, "libs", abi, "lib" + self.game_name + ".so" )
		]

		for s, d in zip( srcBuilds, destBuilds ):
			Log.write( "Copying " + s + " -> " + d )
			shutil.copy( s, d )

		# Create target asset directory.
		Log.write( "Creating asset directory..." )
		createDir( os.path.join( self.android_project_root, "assets" ) )
		createDir( os.path.join( self.android_project_root, "assets/PackedContent" ) )

		# Copy all files.
		Log.write( "Copying all files..." )
		if system() == "Windows":
			expr = re.compile(".*/PackedContent/android\\\\(.*)")
		else:
			expr = re.compile(".*/PackedContent/android/(.*)")
		files = glob.glob( "Dist/PackedContent/android/*")
		for file in files:
			if expr.match(file):
				dest = os.path.join( self.android_project_root, "assets/PackedContent/" ) + expr.match(file).group(1) + ".mp3"
				Log.write( "Copying " + file + " -> " + dest )
				shutil.copy(file, dest)

		# Package up using ant.
		Log.write( "Launching ant debug..." )
		self.launch_ant( "debug" )
		Log.write( "Launching ant release..." )
		self.launch_ant( "release" )

	def install( self, _config ):
		Log.write( "Launching ant " + _config + " uninstall" + self.package + "..." )
		self.launch_ant( _config + " uninstall " + self.package )

		Log.write( "Launching ant " + _config + " install..." )
		self.launch_ant( _config + " install" )

	def launch_android_tool( self, _params ):
		env = copy.deepcopy( os.environ )
		android_command = os.path.join( env["ANDROID_SDK"], "tools", "android" ) + " " + _params

		Log.write( "Launching: " + android_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			subprocess.Popen( android_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )

	def launch_ant( self, _params ):
		env = copy.deepcopy( os.environ )
		ant_command = "ant" + " " + _params

		Log.write( "Launching: " + ant_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			subprocess.Popen( ant_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )

