project ( EXTERNAL_PROJECT_PREFIX .. "png" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	files { "./png/**.h", "./png/**.c" }
	includedirs { ".png/", "./zlib/" }
	links {
		"zlib"
	}
