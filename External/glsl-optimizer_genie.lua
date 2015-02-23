if PsyProjectExternalLib( "glsl-optimizer", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )

	configuration "*"
		excludes {
			"./glsl-optimizer/src/glsl/main.cpp"
		}

	configuration "windows-* or linux-gcc or linux-clang"
		files { 
			"./glsl-optimizer/src/glsl/**.h",
			"./glsl-optimizer/src/glsl/**.c",
			"./glsl-optimizer/src/glsl/**.cpp",
			"./glsl-optimizer/src/mesa/**.h",
			"./glsl-optimizer/src/mesa/**.c",
			"./glsl-optimizer/src/mesa/**.cpp",
			"./glsl-optimizer/src/util/**.h",
			"./glsl-optimizer/src/util/**.c",
			"./glsl-optimizer/src/util/**.cpp",
		}

		includedirs { 
			"./glsl-optimizer/include",
			"./glsl-optimizer/src",
			"./glsl-optimizer/src/mesa",
		}

	configuration "linux-gcc or linux-clang"
		files { 
			"./glsl-optimizer/src/getopt/**.h",
			"./glsl-optimizer/src/getopt/**.c",
			"./glsl-optimizer/src/getopt/**.cpp",
		}
end
