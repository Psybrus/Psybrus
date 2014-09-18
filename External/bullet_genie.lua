	project "External_BulletPhysics"
		
	kind "StaticLib"
	language "C++"
	includedirs {
		"bullet/src",
	}
	files {
		"bullet/src/BulletCollision/**.cpp",
		"bullet/src/BulletCollision/**.h",
		"bullet/src/BulletDynamics/**.cpp",
		"bullet/src/BulletDynamics/**.h",
		"bullet/src/BulletSoftBody/**.cpp",
		"bullet/src/BulletSoftBody/**.h",
		"bullet/src/LinearMath/**.cpp",
		"bullet/src/LinearMath/**.h",
		"bullet/src/vectormath/**.cpp",
		"bullet/src/vectormath/**.h",
		"bullet/src/*.h",
	}