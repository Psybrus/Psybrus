import glob
import os
import subprocess
from platform import system

from Build import *

class BuildMSBuild( Build ):
	def __init__( self, _root_path ):
		Build.__init__( self, _root_path )

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
		self.launch(  self.findSolution() + " /t:Build /property:Configuration=" + _config + " /property:Platform=x64" )

	def launch( self, _params ):
		env = os.environ

		msbuild_command = "\"" + self.tool + "\" " + _params + " /verbosity:minimal /nologo /maxcpucount:4"

		print "Launching: " + msbuild_command

		try:
			subprocess.Popen( msbuild_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			print "Build cancelled. Exiting."
			exit(1)
