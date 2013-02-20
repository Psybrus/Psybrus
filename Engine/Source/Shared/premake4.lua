project "Engine_Shared"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp", "./**.inl" }
	includedirs { "./", "../Platforms/Windows/" }

	-- External includes.
	includedirs { "../../../External/bullet-2.81-rev2613/src" }
	includedirs { "../../../External/enet/include" }
	includedirs { "../../../External/freetype/include" }
	includedirs { "../../../External/glew/include" }
	includedirs { "../../../External/jsoncpp/include" }
	includedirs { "../../../External/libb64/include" }
	includedirs { "../../../External/mongoose" }
	includedirs { "../../../External/ogg/include" }
	includedirs { "../../../External/openal/include" }
	includedirs { "../../../External/pcre" }
	includedirs { "../../../External/png" } -- TODO: Fix this properly.
	includedirs { "../../../External/squish" } -- TODO: Fix this properly.
	includedirs { "../../../External/tremor" } -- TODO: Fix this properly.
	includedirs { "../../../External/zlib" } -- TODO: Fix this properly.
    includedirs { "../../../External/libcurl/include" }

	-- GLEW config.
	defines { "GLEW_STATIC" }

	configuration "windows"
   		links {
   			-- External libs.
   			"External_BulletPhysics",
   			"External_enet",
   			"External_freetype",
   			"External_glew",
   			"External_jsoncpp",
   			"External_libb64",
   			"External_libcurl",
   			"External_mongoose",
   			"External_ogg",
   			"External_pcre",
   			"External_png",
   			"External_squish",
   			"External_tremor",
   			"External_zlib",
   			"External_libcurl",
   		}
