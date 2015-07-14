if PsyProjectExternalLib( "rg-etc1", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./rg-etc1/**.hpp", "./rg-etc1/**.cpp" }
		includedirs { "./rg-etc1/" }
end
