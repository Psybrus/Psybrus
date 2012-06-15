project "External_portaudio"
	kind "StaticLib"
	language "C"
	defines { "PA_USE_DS=1" }
	files { "./include/**.h", "./src/common/**.c", "./src/common/**.h", "./src/hostapi/dsound/**.*", "./src/os/win/**.c", "./src/os/win/**.h" }
	includedirs { ".", "./include", "./src/common/", "./src/os/win/" }
