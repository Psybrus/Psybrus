#!/usr/bin/python

# Build helper.
import argparse
import os
import sys
import shutil

import Toolset

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--toolset", "-t", type=str, help="Toolset to build " + str(Toolset.getToolsets()), required=True )
parser.add_argument( "--config", "-c", type=str, help="Config to build " + str(Toolset.getConfigs()), required=True )
args = parser.parse_args()



platform = Toolset.findPlatform( args.toolset )
if platform == None:
	print "Unable to find platform \"" + args.toolset + "\"."
	exit(1)

platform.build_tool.build( args.config )
