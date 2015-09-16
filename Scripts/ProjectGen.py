#!/usr/bin/python

# Project generation helper.
import argparse
import os
from platform import system
import sys

import Toolset
PLATFORMS = Toolset.PLATFORMS

def doBuild( _build_platform ):
	commandLine = ""
	for additionalOption in _build_platform.extra_flags:
		commandLine += additionalOption + " "
	commandLine += "--file=genie.lua --toolchain={0} --boostpath=$BOOST_ROOT --platform={1} --os={2} {3}".format( 
		_build_platform.name, 
		_build_platform.arch, 
		_build_platform.build_type, 
		_build_platform.project_type )

	print "Launching GENie with: " + commandLine

	if system() == "Linux":
		os.system( "./Psybrus/Tools/genie/linux/genie " + commandLine )
	elif system() == "Darwin":
		os.system( "./Psybrus/Tools/genie/darwin/genie " + commandLine )
	elif system() == "Windows":
		commandLine = commandLine.replace( "$BOOST_ROOT", "%BOOST_ROOT%" );
		os.system( "Psybrus\\Tools\\genie\\windows\\genie.exe " + commandLine )

def selectBuild():
	idx = 0
	print "\nSelect build:"
	for build_platform in PLATFORMS:
		print "\t{0}) {1} ({2}, {3})".format( idx + 1, build_platform.desc, build_platform.name, build_platform.arch )
		idx += 1
	pass
	valid = False
	while valid == False:
		sys.stdout.write( "> " )
		selected = sys.stdin.readline()
		try:
			selected = int(selected)
			selected -= 1
			if selected >= 0 and selected < len(PLATFORMS):
				valid = True
		except ValueError:
			pass
	doBuild( PLATFORMS[ selected ] )

# Arg help.
buildHelpString = ""
for build_platform in PLATFORMS:
	buildHelpString += build_platform.name + ", "

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--build", "-b", type=str, help="Build to select (" + buildHelpString + ")" )
args = parser.parse_args()

if args.build == None:
	selectBuild()
	exit(0)
else:
	for build_platform in PLATFORMS:
		if build_platform.name == args.build:
			doBuild( build_platform )
			exit(0)
	print "Invalid build. Please select one of the following:\n\t" + buildHelpString
	exit(1)

