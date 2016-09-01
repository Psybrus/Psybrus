if PsyProjectExternalLib( "ImGuizmo", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { 
			"./ImGuizmo/ImGuizmo.h", 
			"./ImGuizmo/ImGuizmo.cpp", 
		}
		includedirs { 
			"./ImGuizmo",
			"./imgui/"
		}

	configuration "winphone-*"
		defines { 
			"IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS=1",
			"IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS=1"
		}
end
