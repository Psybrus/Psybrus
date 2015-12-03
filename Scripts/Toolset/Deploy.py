class Deploy():
	def __init__( self, _platform ):
		self.platform = _platform

	def package_build( self, _config ):
		# Run importer for platform.
		self.platform.run_import()

		# Generate build.
		self.platform.build_tool.generate()
		self.platform.build_tool.build( _config )

	def install( self, _config ):
		print "install unimplemented!"

