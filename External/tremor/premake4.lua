project "External_tremor"
	kind "StaticLib"
	language "C"
	files { "./**.h", "./**.c" }
		excludes { "ivorbisfile_example.c", "iseeking_example.c" }
	includedirs { ".", "../ogg/include" }
