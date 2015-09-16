import copy
import glob
import re
import os
import shutil
import subprocess
from platform import system

from Deploy import *

import Log

class DeployAndroid( Deploy ):
	def __init__( self, _platform ):
		Deploy.__init__( self, _platform )

		self.android_project_root = os.path.join( "Build", _platform.project_type + "-" + _platform.name, "project" )

	def package_build( self, _config ):
		Deploy.package_build( self, _config )

		# Create target asset directory.
		Log.write( "Creating asset directory..." )
		try:
			os.mkdir( os.path.join( self.android_project_root, "assets" ) )
		except:
			pass

		try:
			os.mkdir( os.path.join( self.android_project_root, "assets/PackedContent" ) )
		except:
			pass

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
				Log.write( "Copying " + file + " to " + dest )
				shutil.copy(file, dest)

		# Package up using ant.
		Log.write( "Launching ant debug..." )
		self.launch_ant( "release" )
		Log.write( "Launching ant release..." )
		self.launch_ant( "release" )

	def install( self, _config ):
		Log.write( "Launching ant " + _config + " install..." )
		self.launch_ant( _config + " install" )

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

