if PsyProjectExternalLib( "remotery", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { 
			"./Remotery/lib/Remotery.h",
			"./Remotery/lib/Remotery.c",
		}
		defines {
			"RMT_USE_OPENGL=1",
		}
		includedirs { "./Remotery/lib" }

	configuration "android-* or html5-*"
		defines {
			"RMT_ENABLED=0",
		}
end
