import subprocess
import os
import platform

# Builds.
BUILDS = [
	(	
		"Android ARM (GCC 4.9)", 
		"android-gcc-arm", 
		"x32",
		"linux",
		"gmake",
		[]
	),
	( 
		"HTML5 (Clang)", 
		"asmjs", 
		"x32",
		"linux",
		"gmake",
		[]
	),
	( 
		"Linux (Clang 3.5)", 
		"linux-clang", 
		"x64",
		"linux",
		"gmake", 
		[]
	),
	( 
		"OSX (Clang)",
		"osx-clang", 
		"x64",
		"macosx",
		"gmake", 
		[]
	),	
	(
		"Windows (VS2013)", 
		"windows-vs-v120", 
		"x64",
		"windows", 
		"vs2013", 
		[]
	),
	( 
		"Windows (VS2015)", 
		"windows-vs-v140", 
		"x64",
		"windows",
		"vs2015", 
		[]
	),
	( 
		"Windows + DX12 (VS2015)", 
		"windows-vs-v140", 
		"x64",
		"windows",
		"vs2015", 
		[ "--with-dx12" ] # TODO: Have this as a subset of togglable options.
	),
]

GMAKE_TOOLS = {
	"Windows" : (
		# Tool
		"%ANDROID_NDK%\\prebuilt\\windows-x86_64\\bin\\make",
		# PATH env.
		"C:\\Windows\\System32",
	),

	"Linux" : (
		# Tool
		"make",
		# PATH env.
		None,
	),

	"Darwin" : (
		# Tool
		"make",
		# PATH env.
		None,
	),
}

def getToolsets():
	ret = []
	for build in BUILDS:
		ret.append(build[1])
	return ret

def getConfigs():
	return [ "debug", "release", "profile", "production" ]

def findBuild( toolset ):
	for build in BUILDS:
		if build[1] == toolset:
			return build
	return None

def buildToolset( toolset, config ):
	# Force to lowercase.
	toolset = toolset.lower()
	config = config.lower()

	# Check validity of toolset.
	if not (toolset in getToolsets()):
		raise Exception( "Must select valid toolset." )

	# Check validity of config.
	if not (config in getConfigs()):
		raise Exception( "Must select validconfig." )

	# Build.
	build = findBuild( toolset )
	if build[4] == "gmake":
		build_gmake( build, config )
	if build[4] == "vs2013":
		build_vs2013( build, config )
	if build[4] == "vs2015":
		build_vs2015( build, config )

def build_gmake( build, config ):
	gmake_tool = GMAKE_TOOLS[ platform.system() ];

	gmake_executable = gmake_tool[0]
	gmake_path_env = gmake_tool[1]

	env = os.environ
	if gmake_path_env != None:
		env[ "PATH" ] = gmake_path_env

	gmake_executable = gmake_executable.replace( "%ANDROID_NDK%", env["ANDROID_NDK"] )
	gmake_command = gmake_executable + " -j5 config=" + config

	print "Launching: " + gmake_command

	cwd = os.getcwd()
	try:
		os.chdir( os.path.join( "Projects", build[4] + "-" + build[1] ) )
		subprocess.Popen( gmake_command, env=env, shell=True ).communicate()
	except KeyboardInterrupt:
		print "Build cancelled. Exiting."
		exit(1)
	finally:
		os.chdir( cwd )




