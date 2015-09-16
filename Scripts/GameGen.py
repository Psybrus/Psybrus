#!/usr/bin/python

# Game generation helper.
import argparse
import os
import sys
import shutil
import glob
import git

from Toolset import Log

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--name", "-n", type=str, help="Game name. Will create in current working directory.", required=True )
parser.add_argument( "--branch", "-b", type=str, help="Psybrus branch name to checkout." )
args = parser.parse_args()

def recurseFiles( path, func, args ):
	files = glob.glob( path )
	for file in files:
		if os.path.isdir( file ):
			recurseFiles( os.path.join( file, "*" ), func, args )
		else:
			func( file, args )

# Check name has been specified.
# TODO: Validate name has no spaces in it.
if args.name == None:
	Log.write( "ERROR: No -n/--name specified." )
	exit(1)
else:

	# Validate path foesn't exists.
	if os.path.exists( args.name ) == True:
		Log.write( "ERROR: Game \"" + args.name + "\" exists in current directory. Can't create." )
		exit(1)

	# Paths + URLs
	SCRIPTS_PATH = os.path.dirname( sys.argv[0] )
	PSYBRUS_URL = "https://github.com/Psybrus/Psybrus" # TODO: Get using git remote?
	PSYBRUS_REV = git.rev_parse( os.path.dirname( sys.argv[0] ) )
	TEMPLATEGAME_PATH = os.path.join( SCRIPTS_PATH, "..", "TemplateGame" )

	try:
		Log.write( "Copying template..." )
		shutil.copytree( TEMPLATEGAME_PATH, args.name )

		Log.write( "Replacing \"TemplateGame\" with \"" + args.name + "\"..." )

		def replaceContents( file, args ):
			fileString = open( file ).read()
			if args[0] in fileString:
				fileString = fileString.replace( args[0], args[1] )
				f = open( file, 'w' )
				f.write( fileString )
				f.flush()
				f.close()
		recurseFiles( args.name, replaceContents, ( "TemplateGame", args.name ) )

		Log.write( "Initialising repo..." )
		git.init( args.name )

		Log.write( "Committing initial files..." )
		git.stage( args.name, '.' )
		git.commit( args.name, "Initial game files." )

		Log.write( "Adding Psybrus submodule..." )
		git.submodule_add( args.name, PSYBRUS_URL )

		if args.branch == None:
			Log.write( "Checking out to revision " + PSYBRUS_REV + "..." )
			git.checkout( os.path.join( args.name, "Psybrus" ), PSYBRUS_REV )
		else:
			Log.write( "Checking out to branch " + args.branch + "..." )
			git.checkout( os.path.join( args.name, "Psybrus" ), args.branch )

		Log.write( "Committing submodules..." )
		git.stage( args.name, '.' )
		git.commit( args.name, "Initial Psybrus submodule at revision " + PSYBRUS_REV + "." )

		Log.write( "Updating submodules..." )
		git.submodule_update( args.name )

		Log.write( "Done!" )

	finally:
		pass
