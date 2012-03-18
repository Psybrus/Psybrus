project "Engine_IntegrationGwen"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp" }
	includedirs { "./", "../Shared/", "../Platforms/Windows/" }

	includedirs {
 		"../../../External/gwen/include",
 		"../../../External/jsoncpp/include",
	}

	configuration "windows"
   		links {
   			-- External libs.
   			"External_gwen",
   		}
