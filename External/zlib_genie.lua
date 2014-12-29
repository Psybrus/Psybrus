if PsyProjectExternalLib( "zlib", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./zlib/**.h", "./zlib/**.c" }
		includedirs { "./zlib/" }
		defines { "verbose=-1" }
		if ( EXTERNAL_PROJECT_KIND == "SharedLib" ) then
			defines { "ZLIB_DLL=1" }
		end
end
