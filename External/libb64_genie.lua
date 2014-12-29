if PsyProjectExternalLib( "libb64", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./libb64/src/**.h", "./libb64/src/**.c" }
		includedirs { "./libb64/include" }
end
