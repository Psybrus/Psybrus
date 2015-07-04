if PsyProjectExternalLib( "webby", "C" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or linux-* or osx-*"
		files { "./webby/**.h", "./webby/**.c" }
		includedirs { "./webby/" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
end
