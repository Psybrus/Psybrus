if PsyProjectExternalLib( "enet", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./enet/**.h", "./enet/**.c" }
		excludes { "./enet/unix.c" }
		includedirs { "./enet/include" }
		links {
			"ws2_32",
			"IPHlpApi",
			"winmm"
		}
end
