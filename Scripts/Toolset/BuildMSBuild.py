import copy
import glob
import os
import subprocess
from platform import system

from Build import *

import Log

class BuildMSBuild( Build ):
	def __init__( self, _root_path, _platform ):
		Build.__init__( self, _root_path, _platform )

		if system() == "Windows":
			import _winreg
			toolsVersionKeys = [
				r"SOFTWARE\Microsoft\MSBuild\ToolsVersions\14.0",
				r"SOFTWARE\Microsoft\MSBuild\ToolsVersions\4.0",
				r"SOFTWARE\Microsoft\MSBuild\ToolsVersions\3.5",
				r"SOFTWARE\Microsoft\MSBuild\ToolsVersions\2.0"
			]

			tool_root = None
			for toolsVersionKey in toolsVersionKeys:
				with _winreg.OpenKey( _winreg.HKEY_LOCAL_MACHINE, toolsVersionKey, 0, _winreg.KEY_READ ) as key:
					tool_root, i = _winreg.QueryValueEx( key, "MSBuildToolsPath" )
					break

			self.tool = os.path.join( tool_root, "MSBuild.exe" )
		self.root_path = _root_path

	def findSolution( self ):
		slns = glob.glob( os.path.join( self.root_path, "*.sln" ) )
		for sln in slns:
			return sln
		raise Exception( "No solution found in \"" + self.root_path + "\"" )

	def clean( self ):
		self.launch( "/t:Clean" )

	def build( self, _config ):
		archs = {
			"x32" : "Win32",
			"x64" : "x64"
		}

		self.launch(  self.findSolution() + " /t:Build /property:Configuration=" + _config + " /property:Platform=" + archs[ self.platform.arch ] )

	def launch( self, _params ):
		env = copy.deepcopy( os.environ )

		msbuild_command = "\"" + self.tool + "\" " + _params + " /verbosity:minimal /nologo /maxcpucount:4"

		Log.write( "Launching: " + msbuild_command )

		try:
			subprocess.Popen( msbuild_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
