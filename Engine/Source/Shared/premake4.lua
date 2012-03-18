project "Engine_Shared"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp", "./**.inl" }
	includedirs { "./", "./**", "../Platforms/Windows/**" }

	-- External includes.
	includedirs { "../../../External/enet/include" }
	includedirs { "../../../External/freetype/include" }
	includedirs { "../../../External/glew/include" }
	includedirs { "../../../External/jsoncpp/include" }
	includedirs { "../../../External/libb64/include" }
	includedirs { "../../../External/ogg/include" }
	includedirs { "../../../External/openal/include" }
	includedirs { "../../../External/png" } -- TODO: Fix this properly.
	includedirs { "../../../External/squish" } -- TODO: Fix this properly.
	includedirs { "../../../External/tremor" } -- TODO: Fix this properly.
	includedirs { "../../../External/zlib" } -- TODO: Fix this properly.

	-- GLEW config.
	defines { "GLEW_STATIC" }

	configuration "windows"
   		links {
   			-- External libs.
   			"External_Box2D_v2.2.1",
   			"External_enet",
   			"External_freetype",
   			"External_glew",
   			"External_jsoncpp",
   			"External_libb64",
   			"External_ogg",
   			"External_png",
   			"External_squish",
   			"External_tremor",
   			"External_zlib",
   		}
