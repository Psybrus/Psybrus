project "External_jsoncpp"
	kind "StaticLib"
	language "C++"
	files { "./src/lib_json/**.h", "./src/lib_json/**.cpp" }
	includedirs { "./include" }
