if PsyProjectExternalLib( "imgui", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { 
			"./imgui/imgui.h", 
			"./imgui/imgui_internal.h", 
			"./imgui/imgui.cpp",
			"./imgui/imgui_draw.cpp",
			"./imgui/stb_rect_pack.h", 
			"./imgui/stb_textedit.h", 
			"./imgui/stb_truetype.h"
		}
		includedirs { 
			"./imgui/"
		}
end
