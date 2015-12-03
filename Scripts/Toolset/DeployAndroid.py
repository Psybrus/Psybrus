import copy
import glob
import re
import os
import shutil
import subprocess
from platform import system

from Deploy import *

import GameConfig
import Log

def createDir( dir ):
	try:
		os.mkdir( dir )
	except:
		pass

class DeployAndroid( Deploy ):
	def __init__( self, _platform ):
		Deploy.__init__( self, _platform )

		self.android_project_root = os.path.join( "Build", _platform.project_type + "-" + _platform.name, "project" )

	def package_build( self, _config ):
		# Load in config.
		self.game_config = GameConfig.load()
		self.game_name = self.game_config[ "name" ]
		self.game_full_name = self.game_config[ "fullname" ]
		self.android_config = self.game_config[ "android" ]
		self.package = self.android_config[ "package" ]
		self.sdk_version = self.android_config[ "sdk_version" ]
		self.ndk_version = self.android_config[ "ndk_version" ]

		# Tables for lookup.
		self.configs = {
			"debug" : "Debug",
			"release" : "Release",
			"profile" : "Profile",
			"production" : "Production",
		}

		self.abis = {
			"android-gcc-arm" : "armeabi-v7a",
			"android-gcc-x86" : "x86"
		}

		Deploy.package_build( self, _config )

		# TODO: Kill adb and delete project folder?

		# Grab keystore.
		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			keystore_path = self.android_config["keystore"]
			if os.path.exists(keystore_path) == False:
				keystore_path = os.path.join( "../../..", keystore_path )
				if os.path.exists(keystore_path) == False:
					raise Exception( "Unable to find keystore file." )
		except:
			Log.write( "WARNING: No keystore in game config." )
			keystore_path = None
		finally:
			os.chdir( cwd )


		# Create project directory.
		Log.write( "Creating project directory..." )
		createDir( self.android_project_root )
		createDir( os.path.join( self.android_project_root, "assets" ) )
		createDir( os.path.join( self.android_project_root, "assets", "PackedContent" ) )
		createDir( os.path.join( self.android_project_root, "libs" ) )
		for abi_key in self.abis:
			abi = self.abis[ abi_key ]
			createDir( os.path.join( self.android_project_root, "libs", abi ) )
		createDir( os.path.join( self.android_project_root, "res" ) )
		createDir( os.path.join( self.android_project_root, "res", "values" ) )
		createDir( os.path.join( self.android_project_root, "src" ) )

		# Append project properties.
		self.append_project_properties();

		# Setup manifest & strings.
		self.write_manifest()
		self.write_strings()

		# Update project.
		target = "android-" + self.sdk_version
		self.launch_android_tool( "update project -p ./ -t \"" + target + "\" -n \"" + self.game_name + "\"" )

		# Copy build(s) in to target directory.
		abi = self.abis[ self.platform.name ]

		srcBuilds = [
			os.path.join( self.android_project_root, "..", "bin", "lib" + self.game_name + "-" + self.configs[ _config ] + ".so" )
		]

		destBuilds = [
			os.path.join( self.android_project_root, "libs", abi, "lib" + self.game_name + ".so" )
		]

		for s, d in zip( srcBuilds, destBuilds ):
			Log.write( "Copying " + s + " -> " + d )
			shutil.copy( s, d )

		# Setup gdb.
		if _config != "production":
			self.write_gdb_config()

		# Copy all files.
		Log.write( "Copying all files..." )
		if system() == "Windows":
			expr = re.compile(".*/PackedContent/android\\\\(.*)")
		else:
			expr = re.compile(".*/PackedContent/android/(.*)")
		files = glob.glob( "Dist/PackedContent/android/*")
		for file in files:
			if expr.match(file):
				dest = os.path.join( self.android_project_root, "assets/PackedContent/" ) + expr.match(file).group(1) + ".mp3"
				Log.write( "Copying " + file + " -> " + dest )
				shutil.copy(file, dest)

		# Package up using ant.
		Log.write( "Launching ant debug..." )
		self.launch_ant( "debug" )

		#apk_path = os.path.join( "bin", self.game_config[ "name" ] + "-debug.apk" )
		#self.sign_build( keystore_path, apk_path )

		Log.write( "Launching ant release..." )
		self.launch_ant( "release" )

		if keystore_path != None:
			apk_path = os.path.join( "bin", self.game_config[ "name" ] + "-release-unsigned.apk" )
			self.sign_build( keystore_path, apk_path )

	def install( self, _config ):
		return
		antConfig = "debug"
		if _config == "production":
			antConfig + "release"

		Log.write( "Launching ant " + antConfig + " uninstall" + self.package + "..." )
		self.launch_ant( antConfig + " uninstall " + self.package )

		Log.write( "Launching ant " + antConfig + " install..." )
		self.launch_ant( antConfig + " install" )

	def append_project_properties( self ):
		if self.android_config.has_key( "references" ):
			# Update references
			target = "android-" + self.sdk_version
			for reference in self.android_config[ "references" ]:
				self.launch_android_tool( "update project -p " + os.path.join( "../../..", reference ) + " -t " + target )

			# Add references to project.properties
			ref_idx = 1
			for reference in self.android_config[ "references" ]:
				cwd = os.getcwd()
				try:
					echo_command = "echo android.library.reference." + str(ref_idx) + "=../../../" + reference + " > project.properties"
					os.chdir( self.android_project_root )
					subprocess.Popen( echo_command, shell=True ).communicate()
					ref_idx += 1

				except KeyboardInterrupt:
					Log.write( "Build cancelled. Exiting." )
					exit(1)
				finally:
					os.chdir( cwd )

		
		pass

	def write_manifest( self ):
		with open( os.path.join( self.android_project_root, "AndroidManifest.xml" ), "w+" ) as manifestFile:
			manifestFile.write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" )
			manifestFile.write( "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n" )
			manifestFile.write( "          package=\"" + self.android_config["package"] + "\"\n" )
			manifestFile.write( "          android:versionCode=\"" + self.android_config["version_code"] + "\"\n" )
			manifestFile.write( "          android:versionName=\"" + self.android_config["version_name"] + "\">\n" )
			manifestFile.write( "  <uses-sdk android:minSdkVersion=\"" + self.android_config["sdk_version"] + "\" />\n" )

			for permission in self.android_config[ "permissions" ]:
				manifestFile.write( "  <uses-permission android:name=\"" + permission + "\" />\n" )

			manifestFile.write( "  <uses-feature android:glEsVersion=\"" + self.android_config["es_version"] + "\" />\n" )
			manifestFile.write( "  <application android:label=\"@string/app_name\"\n" )
			manifestFile.write( "               android:debuggable=\"true\">\n" )

			if self.android_config.has_key( "metadata" ):
				metadata = self.android_config[ "metadata" ]
				for metadata_key in metadata:
					metadata_value = metadata[ metadata_key ]
					manifestFile.write( "  <meta-data android:name=\"" + metadata_key + "\"\n" )
					manifestFile.write( "             android:value=\"" + metadata_value + "\" />\n" )

			manifestFile.write( "    <activity android:name=\"android.app.NativeActivity\"\n" )
			manifestFile.write( "              android:screenOrientation=\"" + self.android_config["orientation"] + "\"\n" )
			manifestFile.write( "              android:label=\"@string/app_name\">\n" )
			manifestFile.write( "      <meta-data android:name=\"android.app.lib_name\"\n" )
			manifestFile.write( "                 android:value=\"" + self.game_name + "\" />\n" )

			manifestFile.write( "      <intent-filter>\n" )
			manifestFile.write( "        <action android:name=\"android.intent.action.MAIN\" />\n" )
			manifestFile.write( "        <category android:name=\"android.intent.category.LAUNCHER\" />\n" )
			manifestFile.write( "      </intent-filter>\n" )
			manifestFile.write( "    </activity>\n" )
			manifestFile.write( "  </application>\n" )
			manifestFile.write( "</manifest>\n" )

	def write_strings( self ):
		with open( os.path.join( self.android_project_root, "res", "values", "strings.xml" ), "w+" ) as stringsFile:
			stringsFile.write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" )
			stringsFile.write( "<resources>\n" )
			stringsFile.write( "  <string name=\"app_name\">" + self.game_full_name + "</string>\n" )
			if self.android_config.has_key("app_id"):
				stringsFile.write( "  <string name=\"app_id\">" + self.android_config["app_id"] + "</string>\n" )
			stringsFile.write( "</resources>\n" )

	def sign_build( self, key_store_path, apk_path ):
		env = copy.deepcopy( os.environ )

		# To generate keystore:
		# keytool -genkey -v -keystore my-release-key.keystore -alias alias_name -keyalg RSA -keysize 2048 -validity 10000

		# Sign the apk:
		jar_signer = os.path.join( env["JAVA_HOME"], "bin", "jarsigner" )
		jar_signer_command = "\"" + jar_signer + "\"" + " -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore " + key_store_path + " " + apk_path + " " + self.game_config["name"]
		Log.write( "Launching: " + jar_signer_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			subprocess.Popen( jar_signer_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )

		# Align zip:
		# zipalign -v 4 your_project_name-unaligned.apk your_project_name.apk
		pass

	def write_gdb_config( self ):
		env = copy.deepcopy( os.environ )
		abi = self.abis[ self.platform.name ]
		with open( os.path.join( self.android_project_root, "libs", abi, "gdb.setup" ), "w+" ) as gdbFile:
			gdbFile.write( "set solib-search-path ../obj\n" )
			gdbFile.write( "source " + os.path.join( env["ANDROID_NDK"], "prebuilt", "common", "gdb", "common.setup" ) + "\n" )
			gdbFile.write( "directory  ../../../Source ../../../Psybrus/Engine/Source\n" )


	def launch_android_tool( self, _params ):
		env = copy.deepcopy( os.environ )
		android_command = os.path.join( env["ANDROID_SDK"], "tools", "android" ) + " " + _params

		Log.write( "Launching: " + android_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			subprocess.Popen( android_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )

	def launch_ant( self, _params ):
		env = copy.deepcopy( os.environ )
		ant_command = "ant" + " " + _params

		Log.write( "Launching: " + ant_command )

		cwd = os.getcwd()
		try:
			os.chdir( self.android_project_root )
			subprocess.Popen( ant_command, env=env, shell=True ).communicate()
		except KeyboardInterrupt:
			Log.write( "Build cancelled. Exiting." )
			exit(1)
		finally:
			os.chdir( cwd )

