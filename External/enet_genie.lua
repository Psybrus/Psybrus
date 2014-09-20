project ( EXTERNAL_PROJECT_PREFIX .. "enet" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	files { "./enet/**.h", "./enet/**.c" }
	excludes { "./enet/unix.c" }
	includedirs { "./enet/include" }
	links {
		"ws2_32",
		"IPHlpApi",
		"winmm"
	}
