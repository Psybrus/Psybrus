if PsyProjectExternalLib( "webby", "C" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or winphone-* or linux-* or osx-* or android-*"
		files { "./webby/**.h", "./webby/**.c" }
		includedirs { "./webby/" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
end
