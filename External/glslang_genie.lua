if PsyProjectExternalLib( "glslang", "C++11" ) then
	kind ( EXTERNAL_PROJECT_KIND )

	configuration "windows-* or linux-* or osx-*"
		files { 
			"./glslang/glslang/GenericCodeGen/**.h",
			"./glslang/glslang/GenericCodeGen/**.cpp",
			"./glslang/glslang/MachineIndependent/**.h",
			"./glslang/glslang/MachineIndependent/**.cpp",
			"./glslang/glslang/MachineIndependent/gen_glslang_tab.cpp",
			"./glslang/glslang/MachineIndependent/gen_glslang_tab.h",
			"./glslang/OGLCompilersDLL/**.h",
			"./glslang/OGLCompilersDLL/**.cpp",
			"./glslang/glslang/Include/**.h",
			"./glslang/glslang/Public/**.h",
		}

		includedirs { 
			"./glslang/glslang",
		}

	configuration "windows-*"
		files { 
			"./glslang/glslang/OSDependent/Windows/osinclude.h",
			"./glslang/glslang/OSDependent/Windows/ossource.cpp",
		}

		includedirs { 
			"./glslang/glslang/OSDependent/Windows",
		}

		glslangPath = "..\\..\\Psybrus\\External\\glslang\\glslang\\MachineIndependent"
		glslangBisonPath = "..\\..\\Psybrus\\External\\glslang\\Tools\\bison.exe"
		prebuildcommands {
			"@echo Generating gen_glslang_tab.cpp",
			glslangBisonPath .. " -t -v -d " .. glslangPath .. "\\glslang.y",
			"move glslang.tab.c " .. glslangPath .. "\\gen_glslang_tab.cpp",
			"move glslang.tab.h " .. glslangPath .. "\\glslang_tab.cpp.h",
		}

	configuration "linux-* or osx-*"
		files { 
			"./glslang/glslang/OSDependent/Linux/**.h",
			"./glslang/glslang/OSDependent/Linux/**.c",
			"./glslang/glslang/OSDependent/Linux/**.cpp",
		}

		includedirs { 
			"./glslang/glslang/OSDependent/Linux",
		}

		glslangPath = "../../Psybrus/External/glslang/glslang/MachineIndependent"
		prebuildcommands {
			"@echo Generating gen_glslang_tab.cpp",
			"bison -t -v -d " .. glslangPath .. "/glslang.y",
			"mv glslang.tab.c " .. glslangPath .. "/gen_glslang_tab.cpp",
			"mv glslang.tab.h " .. glslangPath .. "/glslang_tab.cpp.h",
		}

end
