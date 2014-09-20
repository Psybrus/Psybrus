project ( EXTERNAL_PROJECT_PREFIX .. "pcre" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C"
	files { 
		"pcre/pcre_byte_order.c",
		"pcre/pcre_chartables.c",
		"pcre/pcre_compile.c",
		"pcre/pcre_config.c",
		"pcre/pcre_dfa_exec.c",
		"pcre/pcre_exec.c",
		"pcre/pcre_fullinfo.c",
		"pcre/pcre_get.c",
		"pcre/pcre_globals.c",
		"pcre/pcre_jit_compile.c",
		"pcre/pcre_maketables.c",
		"pcre/pcre_newline.c",
		"pcre/pcre_ord2utf8.c",
		"pcre/pcre_printint.c",
		"pcre/pcre_refcount.c",
		"pcre/pcre_string_utils.c",
		"pcre/pcre_study.c",
		"pcre/pcre_tables.c",
		"pcre/pcre_ucd.c",
		"pcre/pcre_valid_utf8.c",
		"pcre/pcre_version.c",
		"pcre/pcre_xclass.c",
		"pcre/pcre.h",
		"pcre/config.h"
	}
	includedirs { "./pcre/" }
	defines { "HAVE_CONFIG_H" }