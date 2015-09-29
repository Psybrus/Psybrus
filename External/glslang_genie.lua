if PsyProjectExternalLib( "glslang", "C++11" ) then
	kind ( EXTERNAL_PROJECT_KIND )

	configuration "windows-* or linux-* or osx-*"
		files { 
			"./glslang/glslang/GenericCodeGen/**.h",
			"./glslang/glslang/GenericCodeGen/**.cpp",
			"./glslang/glslang/MachineIndependent/**.h",
			"./glslang/glslang/MachineIndependent/**.cpp",
			"./glslang/glslang/MachineIndependent/glslang_tab.cpp",
			"./glslang/glslang/MachineIndependent/glslang_tab.cpp.h",
			"./glslang/OGLCompilersDLL/**.h",
			"./glslang/OGLCompilersDLL/**.cpp",
			"./glslang/glslang/Include/**.h",
			"./glslang/glslang/Public/**.h",
			"./glslang/SPIRV/**.h",
			"./glslang/SPIRV/**.hpp",
			"./glslang/SPIRV/**.cpp",
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
			"@echo Generating glslang_tab.cpp + glslang_tab.cpp.h",
			glslangBisonPath .. 
				" --defines=" .. glslangPath .. "\\glslang_tab.cpp.h" .. 
				" -t " .. glslangPath .. "\\glslang.y" .. 
				" -o " .. glslangPath .. "\\glslang_tab.cpp"
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
		glslangBisonPath = "bison"
		prebuildcommands {
			"@echo Generating Generating glslang_tab.cpp + glslang_tab.cpp.h",
			glslangBisonPath .. 
				" --defines=" .. glslangPath .. "/glslang_tab.cpp.h" .. 
				" -t " .. glslangPath .. "/glslang.y" .. 
				" -o " .. glslangPath .. "/glslang_tab.cpp"
		}

end
