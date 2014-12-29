if PsyProjectExternalLib( "png", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./png/**.h", "./png/**.c" }
		includedirs { ".png/", "./zlib/" }
		links {
			"zlib"
		}
end
