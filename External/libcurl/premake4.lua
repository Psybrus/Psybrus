project "External_libcurl"
	kind "StaticLib"
	language "C"
	files { "./include/curl/**.h", "./src/**.c" }
	includedirs { "./include" } 
