--
-- manifest.lua
-- Android manifest (and other build cruft) generator for Premake.
-- Copyright (c) 2014 Will Vale and the Premake project
-- Adapted for GENie by Neil Richardson
--

local ndk       = premake.ndk
local project   = premake.project
local config    = premake.config

-- Register the action
newaction {
	trigger     = "ndk-manifest",
	shortname   = "Android NDK manifest",
	description = "Generate manifest and other app build files for Android",

	-- The capabilities of this action
	valid_kinds     = { "WindowedApp", "StaticLib", "SharedLib" },
	valid_languages = { "C", "C++" },
	valid_tools     = {
		cc     = { "gcc" },
	},

	onsolution = function(sln)
		-- Nothing to do here
	end,

	onproject = function(prj)
		-- Not all projects are valid
		if not ndk.isValidProject(prj) then
			return
		end

		-- Need to generate one makefile per configuration
		for cfg in project.eachconfig(prj) do
			if cfg.platform ~= ndk.ANDROID then
				error('The only supported platform for NDK builds is "android"')
			end		

			if cfg.kind == premake.WINDOWEDAPP then
				-- Generate the manifest for application projects only

				-- Define closure to pass config
				function generateManifestCallback(prj)
					ndk.generateManifest(prj, cfg)
				end

				-- Generate the manifest
				premake.generate(prj, ndk.getManifestFilename(prj, cfg), generateManifestCallback)

				-- Produce the activity glue source, if possible.
				if cfg.activity and cfg.baseactivity and cfg.packagename and cfg.basepackagename then
					-- Define closure - we need to tell the project which file to generate.
					function generateActivityCallback(prj)
						ndk.generateActivity(prj, cfg)
					end

					-- Generate activity source
					premake.generate(prj, ndk.getActivityFilename(prj, cfg), generateActivityCallback) 
				end

				-- Export Java source
				for _,v in ipairs(cfg.files) do
					if path.getextension(v) == ndk.JAVA then
						if os.isfile(v) then
							local dst = path.join(ndk.getJavaPath(prj, cfg), path.getname(v))
							printf('Exporting %s...', path.getrelative(os.getcwd(), dst))
							os.copyfile(v, dst)
						end
					elseif not path.iscppfile(v) and not path.iscfile(v) and not path.iscppheader(v) and path.getextension(v) ~= '.lua' then
						if os.isfile(v) then
							local dir = ndk.getAssetPath(prj, cfg)
							os.mkdir(dir)
							local dst = path.join(dir, path.getname(v))
							printf('Exporting %s...', path.getrelative(os.getcwd(), dst))
							os.copyfile(v, dst)
						end
					end
				end
			end
		end
	end,

	oncleansolution = function(sln)
		-- Nothing to do
	end,

	oncleanproject = function(prj)
		for cfg in project.eachconfig(prj) do
			if prj.kind == premake.WINDOWEDAPP then
				-- Just clean the entire folder.
				premake.clean.dir(prj, ndk.getProjectPath(prj, cfg))
			end
		end
	end
}

-- Manifest goes in project dirctory
function ndk.getManifestFilename(this, cfg)
	return path.join(ndk.getProjectPath(this, cfg), ndk.MANIFEST)
end

-- Source path for Java files. They need to be under a directory tree based on the package name.
function ndk.getAssetPath(this, cfg)
	-- Asset files live in assets under the build directory
	return path.join(ndk.getProjectPath(this, cfg), 'assets')
end

-- Source path for Java files. They need to be under a directory tree based on the package name.
function ndk.getJavaPath(this, cfg, packagename)
	-- Java files live in src under the build directory
	local p = path.join(ndk.getProjectPath(this, cfg), 'src')
	if packagename then
		local forward = string.explode(packagename, '%.')
		local reverse = {}
		for _,v in ipairs(forward) do
			table.insert(reverse, 1, v)
		end
		p = path.join(p, table.concat(forward, '/'))
	end
	return p
end

-- Make name of Java file relative to the Java source path
function ndk.getActivityFilename(this, cfg)
	local filename = cfg.activity..ndk.JAVA
	return path.join(ndk.getJavaPath(this, cfg, cfg.packagename), filename)
end

-- Determine which version of OpenGL, if any, to link against.
function ndk.getGlesVersion(cfg)
	local links = config.getlinks(cfg, 'system', 'basename')

	-- Specify highest version we linked against
	if table.contains(links, ndk.GLES30) then
		return 0x00030000
	elseif table.contains(links, ndk.GLES20) then
		return 0x00020000
	elseif table.contains(links, ndk.GLES10) then
		return 0x00010000
	end
end

-- Generate manifest file. This describes the application to ant, the Android OS, and the Google Play store.
function ndk.generateManifest(prj, cfg)
	_p('<?xml version="1.0" encoding="utf-8"?>')
	_p('<!-- Android application manifest autogenerated by Premake -->')
	_x(0, '<manifest xmlns:android="http://schemas.android.com/apk/res/android"')
		_x(1, 'package="%s"', prj.packagename or 'com.example.app')
		_x(1, 'android:versionCode="%d" >', cfg.packageversion or 0)
		--_x(1, 'android:versionString="%d">', cfg.packageversion or 0)

		-- Application tag
		_x(1, '<application android:debuggable="%s"', tostring(cfg.flags.Symbols ~= nil))
			_x(2, 'android:theme="@android:style/Theme.NoTitleBar.Fullscreen">')

			-- Contains activity
			_x(2, '<activity android:name="%s.%s"', prj.packagename or 'com.example.app', cfg.activity or 'Activity')
				_x(3,'android:screenOrientation="landscape">')

				-- Launcher intent
				_x(3, '<intent-filter>')
					_p(4, '<action android:name="android.intent.action.MAIN" />')
					_p(4, '<category android:name="android.intent.category.LAUNCHER" />')
				_p(3, '</intent-filter>')
			
			_p(2, '</activity>')
		
		_x(1, '</application>')

	-- SDK version
	if cfg.framework then
		_x(1, '<uses-sdk android:minSdkVersion="%d" />', ndk.getApiLevel(cfg))
	end

	-- Features
	local gles_version = ndk.getGlesVersion(cfg)
	if gles_version then
		_x(1, '<uses-feature android:glEsVersion="0x%08x" />', gles_version)
	end

	-- Permissions
	for _,v in ipairs(cfg.permissions) do
	    _x(1, '<uses-permission android:name="android.permission.%s" />', v)
	end
	 
	_p('</manifest>')
end

-- Generate Java source file for the app package. This is a bit of a hack, and involves copying a Java file from 
-- the project and patching it to contain the correct package path.
function ndk.generateActivity(prj, cfg)
	_p('// Java activity wrapper generated by Premake')
	_p('package %s;', cfg.packagename)
	_p('import %s.%s;', cfg.basepackagename, cfg.baseactivity)
	_p('public class %s extends %s {}', cfg.activity, cfg.baseactivity)
end

