import copy
import os
import subprocess
from platform import system

class Platform():
	def __init__( self, 
			_name, _desc, _platform, 
			_arch, _build_type, _project_type, 
			_build_tool, _deploy_tool,
			_import_config,
			_extra_flags ):
		self.name = _name
		self.desc = _desc
		self.arch = _arch
		self.platform = _platform
		self.build_type = _build_type
		self.project_type = _project_type

		# TODO: deprecate when we get the concept of a SKU.
		self.import_config = _import_config
		# TODO: deprecate eventually.
		self.extra_flags = _extra_flags

		# Create build tool + path.
		root_path = os.path.join( "Projects", _project_type + "-" + _name )
		self.build_tool = _build_tool( root_path, self )

		# Create deploy tool.
		self.deploy_tool = _deploy_tool( self )

	def run_import( self ):
		cwd = os.getcwd()
		try:
			os.chdir( "Dist" )
			importers = [
				"Importer-Release",
				"Importer-Debug",
			]

			# Append .exe for windows.
			if system() == "Windows":
				new_importers = []
				for importer in importers:
					new_importers.append( importer + ".exe" )
				importers = new_importers

			found_importer = None
			if len(importers) == 0:
				raise Exception( "No importers built. Please build one for your host OS." )

			for importer in importers:
				if os.path.exists( importer ):
					found_importer = importer
					break

			if found_importer != None:
				importer_command = found_importer + " -c " + os.path.join( "..", self.import_config )
				print "Launching: " + importer_command
				try:
					env = copy.deepcopy( os.environ )
					subprocess.Popen( importer_command, env=env, shell=True ).communicate()
				except KeyboardInterrupt:
					print "Build cancelled. Exiting."
					exit(1)
		finally:
			os.chdir( cwd )

PLATFORMS = []
