if PsyProjectExternalLib( "RakNet", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or linux-* or osx-* or android-*"
		files { "./RakNet/Source/**.h", "./RakNet/Source/**.cpp" }
		includedirs { "./RakNet/Source/" }

	configuration "android-*"
		defines { "ANDROID" }

	configuration "windows-*"
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
end
