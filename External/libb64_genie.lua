if PsyProjectExternalLib( "libb64" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		language "C"
		files { "./libb64/src/**.h", "./libb64/src/**.c" }
		includedirs { "./libb64/include" }
end
