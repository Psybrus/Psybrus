project "Engine_Windows"
	kind "StaticLib"
	language "C++"
	files { "./**.h", "./**.c", "./**.cpp" }
	includedirs { "../../Shared/", "./" }

	-- Externals.
	includedirs { "../../../../External/enet/include" }
	includedirs { "../../../../External/freetype/include" }
	includedirs { "../../../../External/glew/include" }
	includedirs { "../../../../External/jsoncpp/include" }
	includedirs { "../../../../External/libb64/include" }
	includedirs { "../../../../External/ogg/include" }
	includedirs { "../../../../External/openal/include" }
	includedirs { "../../../../External/png" } -- TODO: Fix this properly.
	includedirs { "../../../../External/squish" } -- TODO: Fix this properly.
	includedirs { "../../../../External/tremor" } -- TODO: Fix this properly.
	includedirs { "../../../../External/zlib" } -- TODO: Fix this properly.

	configuration "windows"
   		links {
   			-- Engine lins.
   			"Engine_Shared",

  		}