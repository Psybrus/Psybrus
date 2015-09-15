import os

class Platform():
	def __init__( self, 
			_name, _desc, _platform, 
			_arch, _build_type, _project_type, 
			_build_tool,
			_extra_flags ):
		self.name = _name
		self.desc = _desc
		self.arch = _arch
		self.platform = _platform
		self.build_type = _build_type
		self.project_type = _project_type

		# TODO: deprecate eventually.
		self.extra_flags = _extra_flags

		# Create build tool + path.
		root_path = os.path.join( "Projects", _project_type + "-" + _name )
		self.build_tool = _build_tool( root_path )

PLATFORMS = []
