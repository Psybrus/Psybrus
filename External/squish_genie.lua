if PsyProjectExternalLib( "squish", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./squish/*.h", "./squish/*.cpp" }
		includedirs { "./squish" }
		defines{ "SQUISH_USE_SSE=2" }
end
