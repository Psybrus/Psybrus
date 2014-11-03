PsyProjectExternalLib( EXTERNAL_PROJECT_PREFIX .. "glew" )
	kind ( EXTERNAL_PROJECT_KIND ) 
	language "C++"
	configuration "windows or linux-gcc or linux-clang"
		files { "./glew/include/**.h", "./glew/src/glew.c" }
		includedirs { "./glew/include" }
		defines { "GLEW_BUILD=1" }
		if ( EXTERNAL_PROJECT_KIND == "StaticLib" ) then
			defines { "GLEW_STATIC=1" }
		end
		links {
			"opengl32"
		}
