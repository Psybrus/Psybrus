PsyProjectExternalLib( "portaudio" )
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		language "C"
		defines { "PA_USE_DS=1", "PA_USE_WASAPI=1", "PA_USE_WDMKS=1" }
		files { 
			"./portaudio/include/**.h",
			"./portaudio/src/common/**.c",
			"./portaudio/src/common/**.h",
			"./portaudio/src/hostapi/dsound/**.*",
			"./portaudio/src/hostapi/wasapi/**.*",
			"./portaudio/src/hostapi/wdmks/**.*",
			"./portaudio/src/os/win/**.c",
			"./portaudio/src/os/win/**.h" 
		}
		includedirs { 
			".", 
			"./portaudio/include", 
			"./portaudio/src/common/", 
			"./portaudio/src/os/win/" 
		}
