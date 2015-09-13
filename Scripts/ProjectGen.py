#!/usr/bin/python

# Project generation helper.
import argparse
import os
import platform
import sys

import Toolset
BUILDS = Toolset.BUILDS

def doBuild( _build ):
	commandLine = ""
	for additionalOption in _build[5]:
		commandLine += additionalOption + " "
	commandLine += "--toolchain={0} --boostpath=$BOOST_ROOT --platform={1} --os={2} {3}".format( _build[1], _build[2], _build[3], _build[4] )

	print "Launching GENie with: " + commandLine

	if platform.system() == "Linux":
		os.system( "./Psybrus/Tools/genie/linux/genie " + commandLine )
	elif platform.system() == "Darwin":
		os.system( "./Psybrus/Tools/genie/darwin/genie " + commandLine )
	elif platform.system() == "Windows":
		commandLine = commandLine.replace( "$BOOST_ROOT", "%BOOST_ROOT%" );
		os.system( "Psybrus\\Tools\\genie\\windows\\genie.exe " + commandLine )

def selectBuild():
	idx = 0
	print "\nSelect build:"
	for build in BUILDS:
		print "\t{0}) {1} ({2}, {3})".format( idx + 1, build[0], build[1], build[2] )
		idx += 1
	pass
	valid = False
	while valid == False:
		sys.stdout.write( "> " )
		selected = sys.stdin.readline()
		try:
			selected = int(selected)
			selected -= 1
			if selected >= 0 and selected < len(BUILDS):
				valid = True
		except ValueError:
			pass
	doBuild( BUILDS[ selected ] )

# Arg help.
buildHelpString = ""
for build in BUILDS:
	buildHelpString += build[1] + ", "

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--build", "-b", type=str, help="Build to select (" + buildHelpString + ")" )
args = parser.parse_args()

if args.build == None:
	selectBuild()
	exit(0)
else:
	for build in BUILDS:
		if build[1] == args.build:
			doBuild( build )
			exit(0)
	print "Invalid build. Please select one of the following:\n\t" + buildHelpString
	exit(1)

