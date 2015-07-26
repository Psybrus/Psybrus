#!/usr/bin/python

import os
import sys
import shutil
import glob
import re

try:
	os.mkdir("assets")
	os.mkdir("assets/PackedContent")
except:
	pass

if sys.platform == "win32" or sys.platform == "win64":
	expr = re.compile(".*/PackedContent/android\\\\(.*)")
else:
	expr = re.compile(".*/PackedContent/android/(.*)")

files = glob.glob("../../../Dist/PackedContent/android/*")
for file in files:
	if expr.match(file):
		dest = "assets/PackedContent/" + expr.match(file).group(1) + ".mp3"
		print "Copying .. " + file + " to " + dest
		shutil.copy(file, dest)
