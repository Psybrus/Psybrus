if PsyProjectExternalLib( "BulletPhysics", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		includedirs {
			"bullet3/src",
		}
		files {
			"bullet3/src/BulletCollision/**.cpp",
			"bullet3/src/BulletCollision/**.h",
			"bullet3/src/BulletDynamics/**.cpp",
			"bullet3/src/BulletDynamics/**.h",
			"bullet3/src/BulletSoftBody/**.cpp",
			"bullet3/src/BulletSoftBody/**.h",
			"bullet3/src/LinearMath/**.cpp",
			"bullet3/src/LinearMath/**.h",
			"bullet3/src/vectormath/**.cpp",
			"bullet3/src/vectormath/**.h",
			"bullet3/src/*.h",
		}
end
