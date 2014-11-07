if PsyProjectExternalLib( "jsoncpp" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		language "C++"
		files { "./jsoncpp/src/lib_json/**.h", "./jsoncpp/src/lib_json/**.cpp" }
		includedirs { "./jsoncpp/include" }
end
