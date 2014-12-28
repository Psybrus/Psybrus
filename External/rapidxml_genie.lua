project ( EXTERNAL_PROJECT_PREFIX .. "rapidxml" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	files { "./rapidxml/**.hpp" }
	includedirs { "./rapidxml/" }
