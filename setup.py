import os


def addEnvironmentVar(key, value):
	print "Setting environment variable \"%s\" to \"%s\"" % (key, value)
	os.system( "setx %s \"%s\"" % (key, value) )



setupText = """
Psybrus Setup
========================================

This will setup some environment vars, and do some basic
checks for software that is required to use and build Psybrus.

"""


print setupText

# Check for VS2012
print "Checking for Microsoft Visual Studio 2012..."
vs110ComnTools = os.getenv("VS110COMNTOOLS", None)
if vs110ComnTools == None:
	print "Can't find Visual Studio 2012. Do you have it installed?"
	exit(1)
else:
	print "...got!"


# Check for DXSDK
print "Checking for DirectX SDK (June 2010)..."
vs110ComnTools = os.getenv("DXSDK_DIR", None)
if vs110ComnTools == None:
	print "Can't find, is DXSDK_DIR set?"
	exit(1)
else:
	print "...got!"


# Check for boost.
print "Checking for Boost..."
boostRoot = os.getenv("BOOST_ROOT", None)
if boostRoot == None:
	print "BOOST_ROOT is not set to Boost root directory (i.e. C:\\Boost)"
	print "Please set this up and run setup again"
	exit(1)
else:
	print "...got!"

# Add Psybrus env var.
addEnvironmentVar("PSYBRUS_SDK", os.getcwd())


# Create project files.
os.system("premake4.exe --os=windows --platform=x64 vs2012")