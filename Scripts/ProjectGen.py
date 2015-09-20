#!/usr/bin/python

# Project generation helper.
import argparse
import os
from platform import system
import sys

import Toolset
PLATFORMS = Toolset.PLATFORMS

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
	PLATFORMS[ selected ].build_tool.generate()

# Arg help.
buildHelpString = ""
for build_platform in PLATFORMS:
	buildHelpString += build_platform.name + ", "

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--toolset", "-t", type=str, help="Toolset to select (" + buildHelpString + ")" )
args = parser.parse_args()

if args.toolset == None:
	selectBuild()
	exit(0)
else:
	for build_platform in PLATFORMS:
		if build_platform.name == args.toolset:
			build_platform.build_tool.generate()
			exit(0)
	print "Invalid toolset. Please select one of the following:\n\t" + buildHelpString
	exit(1)

