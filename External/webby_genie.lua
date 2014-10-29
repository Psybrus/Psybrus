PsyProjectExternalLib( EXTERNAL_PROJECT_PREFIX .. "webby" )
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		language "C"
		files { "./webby/**.h", "./webby/**.c" }
		includedirs { "./webby/" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
