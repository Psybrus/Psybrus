import Log

import os
from platform import system
import sys

class Build():
	def __init__( self, _root_path, _platform ):
		self.root_path = _root_path
		self.platform = _platform

	def generate( self ):
		command_line = ""
		for additional_option in self.platform.extra_flags:
			commandLine += additional_option + " "
		command_line += "--file=genie.lua --toolchain={0} --boostpath=$BOOST_ROOT --platform={1} --os={2} {3}".format( 
			self.platform.name, 
			self.platform.arch, 
			self.platform.build_type, 
			self.platform.project_type )

		Log.write( "Launching GENie with: " + command_line )

		if system() == "Linux":
			os.system( "./Psybrus/Tools/genie/linux/genie " + command_line )
		elif system() == "Darwin":
			os.system( "./Psybrus/Tools/genie/darwin/genie " + command_line )
		elif system() == "Windows":
			command_line = command_line.replace( "$BOOST_ROOT", "%BOOST_ROOT%" );
			os.system( "Psybrus\\Tools\\genie\\windows\\genie.exe " + command_line )


	def clean( self ):
		print "clean unimplemented!"

	def build( self, _config ):
		print "build unimplemented!"
