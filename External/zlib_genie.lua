project ( EXTERNAL_PROJECT_PREFIX .. "zlib" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	files { "./zlib/**.h", "./zlib/**.c" }
	includedirs { "./zlib/" }
	if ( EXTERNAL_PROJECT_KIND == "SharedLib" ) then
		defines { "ZLIB_DLL=1" }
	end