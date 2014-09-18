import glob
import os
import re
import sys

psybrusSdkRoot = os.getenv("PSYBRUS_SDK", None);

if psybrusSdkRoot == None:
	print "Must run setup.py to setup the Psybrus SDK!"
	exit(1)


def getPathFromRoot( targetPath ):
	return ( psybrusSdkRoot + "/" + targetPath).replace( "\\", "/" )


# (Engine\/Source\/Shared\/|Engine\/Source\/Platforms\/.*?\/)
incStripPattern = "(.*?Engine\/Source\/Shared\/|.*?Engine\/Source\/Platforms\/.*?\/|.*?Source\/)(.*)"
incStripProg = re.compile( incStripPattern )

declPattern = "\s(REFLECTION_DECLARE_BASIC_MANUAL_NOINIT\(|REFLECTION_DECLARE_BASE_MANUAL_NOINIT\(|REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT\(|DECLARE_RESOURCE\(|REFLECTION_DECLARE_BASIC\(|REFLECTION_DECLARE_BASE\(|REFLECTION_DECLARE_DERIVED\()\s(.*?)(,|\s\))"
declProg = re.compile( declPattern )
def recurse( startPath, call ):
	paths = glob.glob( startPath + "/*" )
	for path in paths:
		path = path.replace( '\\', '/' )
		if os.path.isfile( path ):
			call( str( path ) )
		else:
			recurse( path, call )

def printPath( path ):
	if path.endswith( ".h" ):
		print path

types = []
includes = []


def parseReflection( path ):
	if path.endswith( ".h" ):
		with open( path ) as f:
			global types
			global includes
			lines = f.readlines()
			for line in lines:
				match = declProg.match( line )
				if match != None:
					reflectionType = match.group(2)
					if reflectionType.startswith( "_" ) == False:
						types.append( reflectionType )
						pathMatch = incStripProg.match( path )
						if pathMatch != None:
							newPath = pathMatch.group(2)
							if ( newPath in includes ) == False:
								includes.append( newPath )
						else:
							raise Exception( "Failed to match path - " + path )

searchPaths = [
	"Engine/Source/Shared",
]

if len( sys.argv ) < 2:
	print "Missing command line args:\n"
	print "python reflection_parse.py GameName "
	exit(1)

gameName = sys.argv[1]
gameSourcePath = "../" + gameName + "/Source"
outputFileName = getPathFromRoot( gameSourcePath ) + "/AutoGenRegisterReflection.cpp"
searchPaths.append ( "../" + gameName + "/Source" )

outputFile = []

for searchPath in searchPaths:
	fullPath = getPathFromRoot( searchPath )
	recurse( fullPath, parseReflection )

for include in includes:
	outputFile.append( "#include \"" + include + "\"\n" )

outputFile.append( "void AutoGenRegisterReflection()\n" )
outputFile.append( "{\n" )
for reflectionType in types:
	outputFile.append( "\t" + reflectionType + "::StaticRegisterClass();\n" )
outputFile.append( "}\n" )


with open( outputFileName, "w+" ) as f:
	f.writelines( outputFile )
