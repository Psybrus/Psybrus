PsyProjectExternalLib( EXTERNAL_PROJECT_PREFIX .. "webby" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	configuration "windows or linux-gcc or linux-clang"
		files { "./webby/**.h", "./webby/**.c" }
		includedirs { "./webby/" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
