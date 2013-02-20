	project "External_BulletPhysics"
		
	kind "StaticLib"
	language "C++"
	includedirs {
		"src",
	}
	files {
		"src/BulletCollision/**.cpp",
		"src/BulletCollision/**.h",
		"src/BulletDynamics/**.cpp",
		"src/BulletDynamics/**.h",
		"src/BulletSoftBody/**.cpp",
		"src/BulletSoftBody/**.h",
		"src/LinearMath/**.cpp",
		"src/LinearMath/**.h",
		"src/vectormath/**.cpp",
		"src/vectormath/**.h",
		"src/*.h",
	}