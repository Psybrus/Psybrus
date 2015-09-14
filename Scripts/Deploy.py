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


