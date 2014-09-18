project "External_jsoncpp"
	kind "StaticLib"
	language "C++"
	files { "./jsoncpp/src/lib_json/**.h", "./jsoncpp/src/lib_json/**.cpp" }
	includedirs { "./jsoncpp/include" }
