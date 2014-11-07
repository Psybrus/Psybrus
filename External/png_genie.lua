if PsyProjectExternalLib( "png" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		language "C"
		files { "./png/**.h", "./png/**.c" }
		includedirs { ".png/", "./zlib/" }
		links {
			"zlib"
		}
end
