project "External_portaudio"
	kind "StaticLib"
	language "C"
	defines { "PA_USE_DS=1", "PA_USE_WASAPI=1", "PA_USE_WDMKS=1" }
	files { "./include/**.h",
	"./src/common/**.c",
	"./src/common/**.h",
	"./src/hostapi/dsound/**.*",
	"./src/hostapi/wasapi/**.*",
	"./src/hostapi/wdmks/**.*",
	"./src/os/win/**.c",
	"./src/os/win/**.h" }
	includedirs { ".", "./include", "./src/common/", "./src/os/win/" }
