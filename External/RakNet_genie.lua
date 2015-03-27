if PsyProjectExternalLib( "RakNet", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or linux-*"
		files { "./RakNet/Source/**.h", "./RakNet/Source/**.cpp" }
		includedirs { "./RakNet/Source/" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
end
