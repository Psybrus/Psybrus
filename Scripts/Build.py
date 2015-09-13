#!/usr/bin/python

# Build helper.
import argparse
import os
import platform
import sys
import shutil

import Toolset

# Parse args.
parser = argparse.ArgumentParser()
parser.add_argument( "--toolset", "-t", type=str, help="Toolset to build " + str(Toolset.getToolsets()), required=True )
parser.add_argument( "--config", "-c", type=str, help="Config to build " + str(Toolset.getConfigs()), required=True )
args = parser.parse_args()


Toolset.buildToolset( args.toolset, args.config )