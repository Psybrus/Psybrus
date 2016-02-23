if PsyProjectExternalLib( "fcpp", "C" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { 
			"./fcpp/cpp.h",
			"./fcpp/cpp1.c",
			"./fcpp/cpp2.c",
			"./fcpp/cpp3.c",
			"./fcpp/cpp4.c",
			"./fcpp/cpp5.c",
			"./fcpp/cpp6.c",
			"./fcpp/cppadd.h",
			"./fcpp/cppdef.h",
			"./fcpp/fpp.h",
			"./fcpp/fpp_pragmas.h",
			"./fcpp/FPP_protos.h",
			"./fcpp/FPPBase.h",
		}
		includedirs { "./fcpp" }
end
