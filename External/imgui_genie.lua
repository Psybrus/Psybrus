if PsyProjectExternalLib( "imgui", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { 
			"./imgui/imgui.h", 
			"./imgui/imgui_internal.h", 
			"./imgui/imgui.cpp",
			"./imgui/imgui_draw.cpp",
			"./imgui/imgui_demo.cpp",
			"./imgui/stb_rect_pack.h", 
			"./imgui/stb_textedit.h", 
			"./imgui/stb_truetype.h"
		}
		includedirs { 
			"./imgui/"
		}

	configuration "winphone-*"
		defines { 
			"IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS=1",
			"IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS=1"
		}
end
