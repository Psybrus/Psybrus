#!/usr/bin/python

# Game generation helper.
import argparse
import os
import platform
import sys
import shutil
import glob
import git

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--name", "-n", type=str, help="Game name. Will create in current working directory.", required=True )
parser.add_argument( "--branch", "-b", type=str, help="Psybrus branch name to checkout." )
args = parser.parse_args()

def log( msg ):
	print msg
	sys.stdout.flush()


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
	log( "ERROR: No -n/--name specified." )
	exit(1)
else:

	# Validate path foesn't exists.
	if os.path.exists( args.name ) == True:
		log( "ERROR: Game \"" + args.name + "\" exists in current directory. Can't create." )
		exit(1)

	# Paths + URLs
	SCRIPTS_PATH = os.path.dirname( sys.argv[0] )
	PSYBRUS_URL = "https://github.com/Psybrus/Psybrus" # TODO: Get using git remote?
	PSYBRUS_REV = git.rev_parse( os.path.dirname( sys.argv[0] ) )
	TEMPLATEGAME_PATH = os.path.join( SCRIPTS_PATH, "..", "TemplateGame" )

	try:
		log( "Copying template..." )
		shutil.copytree( TEMPLATEGAME_PATH, args.name )

		log( "Replacing \"TemplateGame\" with \"" + args.name + "\"..." )

		def replaceContents( file, args ):
			fileString = open( file ).read()
			if args[0] in fileString:
				fileString = fileString.replace( args[0], args[1] )
				f = open( file, 'w' )
				f.write( fileString )
				f.flush()
				f.close()
		recurseFiles( args.name, replaceContents, ( "TemplateGame", args.name ) )

		log( "Initialising repo..." )
		git.init( args.name )

		log( "Committing initial files..." )
		git.stage( args.name, '.' )
		git.commit( args.name, "Initial game files." )

		log( "Adding Psybrus submodule..." )
		git.submodule_add( args.name, PSYBRUS_URL )

		if args.branch == None:
			log( "Checking out to revision " + PSYBRUS_REV + "..." )
			git.checkout( os.path.join( args.name, "Psybrus" ), PSYBRUS_REV )
		else:
			log( "Checking out to branch " + args.branch + "..." )
			git.checkout( os.path.join( args.name, "Psybrus" ), args.branch )

		log( "Committing submodules..." )
		git.stage( args.name, '.' )
		git.commit( args.name, "Initial Psybrus submodule at revision " + PSYBRUS_REV + "." )

		log( "Updating submodules..." )
		git.submodule_update( args.name )

		log( "Done!" )

	finally:
		pass
