if PsyProjectExternalLib( "ThinkGear", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./ThinkGear/**.h", "./ThinkGear/**.c" }
		includedirs { "./ThinkGear" }
end
