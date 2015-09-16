class Deploy():
	def __init__( self, _platform ):
		self.platform = _platform

	def package_build( self, _config ):
		# Run importer for platform.
		self.platform.run_import()

	def install( self, _config ):
		print "install unimplemented!"

