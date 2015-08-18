if PsyProjectExternalLib( "hlsl2glslfork", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )

	configuration "windows-* or linux-* or osx-*"
		files { 
			"./hlsl2glslfork/hlslang/GLSLCodeGen/**.h",
			"./hlsl2glslfork/hlslang/GLSLCodeGen/**.cpp",
			"./hlsl2glslfork/hlslang/Include/**.h",
			"./hlsl2glslfork/hlslang/Include/**.cpp",
			"./hlsl2glslfork/hlslang/MachineIndependent/**.h",
			"./hlsl2glslfork/hlslang/MachineIndependent/**.cpp",
		}

		files {
			"./hlsl2glslfork/hlslang/MachineIndependent/Gen_hlslang.cpp",
			"./hlsl2glslfork/hlslang/MachineIndependent/Gen_hlslang_tab.cpp",
			"./hlsl2glslfork/hlslang/MachineIndependent/hlslang_tab.h",
		}

		includedirs { 
			"./hlsl2glslfork/hlslang", 
			"./hlsl2glslfork/hlslang/MachineIndependent", 
			"./hlsl2glslfork/include", 
		}

	configuration "windows-*"
		files { 
			"./hlsl2glslfork/hlslang/OSDependent/Windows/**.h", 
			"./hlsl2glslfork/hlslang/OSDependent/Windows/**.cpp", 
		}

		includedirs { 
			"./hlsl2glslfork/hlslang/OSDependent/Windows", 
		}

	configuration "linux-*"
		files { 
			"./hlsl2glslfork/hlslang/OSDependent/Linux/**.h", 
			"./hlsl2glslfork/hlslang/OSDependent/Linux/**.cpp", 
		}

		includedirs { 
			"./hlsl2glslfork/hlslang/OSDependent/Linux", 
		}

	configuration "osx-*"
		files { 
			"./hlsl2glslfork/hlslang/OSDependent/Mac/**.h", 
			"./hlsl2glslfork/hlslang/OSDependent/Mac/**.cpp", 
		}

		includedirs { 
			"./hlsl2glslfork/hlslang/OSDependent/Mac", 
		}

		-- prebuildcommands {
		-- 	"$(SILENT) echo Executing Bison on hlslang.y",
		-- 	"$(SILENT) export BISON_SIMPLE=../../Psybrus/External/hlsl2glslfork/tools/bison.simple",
		-- 	"$(SILENT) export BISON_HAIRY=../../Psybrus/External/hlsl2glslfork/tools/bison.simple",
		-- 	"$(SILENT) bison -o ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/hlslang_tab.cpp -d -t -v ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/hlslang.y",
		-- 	"$(SILENT) mv ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/hlslang_tab.hpp ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/hlslang_tab.h",
		-- }

		-- prebuildcommands {
		-- 	"$(SILENT) echo Executing Flex on hlslang.l",
		-- 	"$(SILENT) flex ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/hlslang.l",
		-- 	"$(SILENT) mv Gen_hlslang.cpp ../../Psybrus/External/hlsl2glslfork/hlslang/MachineIndependent/Gen_hlslang.cpp",
		-- }

end
