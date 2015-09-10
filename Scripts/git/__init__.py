import subprocess
import os

def init( path ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		return subprocess.check_output(['git', 'init'])
	finally:
		os.chdir( cwd )

def checkout( path, rev ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		return subprocess.check_output(['git', 'checkout', rev])
	finally:
		os.chdir( cwd )

def commit( path, msg ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		return subprocess.check_output(['git', 'commit', '-m \"' + msg + '\"'])
	finally:
		os.chdir( cwd )


def stage( path, files ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		output = []
		for file in files:
			output.append( subprocess.check_output(['git', 'stage', file ]) )
		return output
	finally:
		os.chdir( cwd )

def submodule_add( path, source ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		return subprocess.check_output(['git', 'submodule', 'add', source ])
	finally:
		os.chdir( cwd )

def submodule_update( path ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		return subprocess.check_output(['git', 'submodule', 'update', '--init', '--recursive' ])
	finally:
		os.chdir( cwd )

def rev_parse( path ):
	cwd = os.getcwd()
	try:
		os.chdir( path )
		rev = subprocess.check_output(['git', 'rev-parse', 'HEAD'])
		if rev != None:
			rev = rev[:-1]
		return rev
	finally:
		os.chdir( cwd )
