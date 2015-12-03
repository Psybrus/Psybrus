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

class DeployHTML5( Deploy ):
	def __init__( self, _platform ):
		Deploy.__init__( self, _platform )

	def package_build( self, _config ):
		Deploy.package_build( self, _config )

	def install( self, _config ):
		print "install unimplemented! TODO: Upload to server via scp/sftp/ftp/similar."

