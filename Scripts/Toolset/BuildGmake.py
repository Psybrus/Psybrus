import copy
import os
from platform import system
import subprocess

from Build import *

import Log

class BuildGmake( Build ):
	def __init__( self, _root_path, _platform ):
		Build.__init__( self, _root_path, _platform )

		PlatformSpecificGmake = {
			"Windows" : (
				"%ANDROID_NDK%\\prebuilt\\windows-x86_64\\bin\\make",
				"C:\\Windows\\System32" ),

			"Linux" : (
				"make", 
				None ),

			"Darwin" : (
				"make",
				None ),
		}

		platform_specific_gmake = PlatformSpecificGmake[ system() ]
		self.tool = platform_specific_gmake[0]
		self.path_env = platform_specific_gmake[1]
		self.root_path = _root_path

		# TODO: Should make generic to replace all env vars found.
		env = copy.deepcopy( os.environ )
		if env.has_key( "ANDROID_NDK" ):
			self.tool = self.tool.replace( "%ANDROID_NDK%", env["ANDROID_NDK"] )

	def clean( self ):
		self.launch( "clean" )

	def build( self, _config ):
		self.launch( " -j5 config=" + _config )

	def launch( self, _params ):
		env = copy.deepcopy( os.environ )
		if self.path_env != None:
			env[ "PATH" ] = self.path_env

		gmake_command = self.tool + " " + _params

		Log.write( "Launching: " + gmake_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.root_path )
			subprocess.Popen( gmake_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )
