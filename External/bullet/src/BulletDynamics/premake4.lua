	project "BulletDynamics"
		
	kind "StaticLib"
	language "C++"
	targetdir "../../lib"
	includedirs {
		"..",
	}
	files {
		"**.cpp",
		"**.h"
	}