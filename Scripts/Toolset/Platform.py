class Platform():
	def __init__( self, 
			_name, _desc, _platform, 
			_arch, _build_type, _project_type, 
			_extra_flags ):
		self.name = _name
		self.desc = _desc
		self.arch = _arch
		self.platform = _platform
		self.build_type = _build_type
		self.project_type = _project_type
		self.extra_flags = _extra_flags

PLATFORMS = []
