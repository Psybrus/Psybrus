if PsyProjectExternalLib( "imgui", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./imgui/*.h", "./imgui/*.cpp" }
		includedirs { 
			"./imgui-config/",
			"./imgui/" }
end
