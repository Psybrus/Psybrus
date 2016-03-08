import glob
import os
import re
import sys

psybrusSdkRoot = "../Psybrus" #vos.getenv("PSYBRUS_SDK", None);

def getPathFromRoot( targetPath ):
	return ( psybrusSdkRoot + "/" + targetPath).replace( "\\", "/" )


def log( string ):
	#print string
	pass

# (Engine\/Source\/Shared\/|Engine\/Source\/Platforms\/.*?\/)
incStripPattern = "(.*?Engine\/Source\/Shared\/|.*?Engine\/Source\/Platforms\/.*?\/|.*?Source\/)(.*)"
incStripProg = re.compile( incStripPattern )

declPattern = "\s*(REFLECTION_DECLARE_BASIC_MANUAL_NOINIT\(|REFLECTION_DECLARE_BASE_MANUAL_NOINIT\(|REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT\(|DECLARE_RESOURCE\(|REFLECTION_DECLARE_BASIC\(|REFLECTION_DECLARE_BASE\(|REFLECTION_DECLARE_DERIVED\()\s*(.*?)(,|\s*\))"
declProg = re.compile( declPattern )
def recurse( startPath, call ):
	log("Searching: " + startPath)
	paths = glob.glob( startPath + "/*" )
	paths.sort()
	for path in paths:
		path = path.replace( '\\', '/' )
		if os.path.isfile( path ):
			call( str( path ) )
		else:
			recurse( path, call )

def printPath( path ):
	if path.endswith( ".h" ):
		log(path)

types = []
includes = []


def parseReflection( path ):
	if path.endswith( ".h" ):
		with open( path ) as f:
			log(" - " + path)
			global types
			global includes
			lines = f.readlines()
			for line in lines:
				match = declProg.match( line )
				if match != None:
					reflectionType = match.group(2)
					log(" - - FOUND: " + reflectionType)
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
	"../../Psybrus/Engine/Source/Shared",
]

if len( sys.argv ) < 2:
	print "Missing command line args:\n"
	print "python reflection_parse.py GameFolder "
	exit(1)

gameFolder = sys.argv[1]
gameSourcePath = "../../Source/"
outputFileName = gameSourcePath + "AutoGenRegisterReflection.cpp"
searchPaths.append ( gameSourcePath )

outputFile = []

for searchPath in searchPaths:
	fullPath =  searchPath 
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
