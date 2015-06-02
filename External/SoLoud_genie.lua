local WITH_SDL = 0
local WITH_SDL_STATIC = 0
local WITH_SDL2_STATIC = 0
local WITH_PORTAUDIO = 0
local WITH_OPENAL = 0
local WITH_XAUDIO2 = 0
local WITH_WINMM = 0
local WITH_WASAPI = 0
local WITH_OSS = 0
local WITH_LIBMODPLUG = 0
local WITH_PORTMIDI = 0
local WITH_TOOLS = 0
local WITH_NULL = 1

if (os.is("Windows")) then
	WITH_WINMM = 1
	WITH_WASAPI = 1
else 
	WITH_SDL2_STATIC = 1
end

-- Hack io asmjs.
if _OPTIONS[ "toolchain" ] == "asmjs" then
	WITH_OSS = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_SDL = 0
	WITH_SDL_STATIC = 1
	WITH_SDL2_STATIC = 0
end

-- Hack android.
if _OPTIONS[ "toolchain" ] == "android-clang-arm" or
   _OPTIONS[ "toolchain" ] == "android-gcc-arm" then
	WITH_OSS = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_SDL = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
end


-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

local sdl_root       = "./sdl"
local portmidi_root  = "./portmidi"
local dxsdk_root     = os.getenv("DXSDK_DIR") and os.getenv("DXSDK_DIR") or "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"
local portaudio_root = "./portaudio"
local openal_root    = "./openal"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

local sdl_include       = sdl_root .. "/include"      
local sdl_lib           = sdl_root .. "/lib"
local portmidi_include  = portmidi_root .. "/pm_common"
local portmidi_debug    = portmidi_root .. "/debug"
local portmidi_release  = portmidi_root .. "/release"
local dxsdk_include     = dxsdk_root .. "/include"
local portaudio_include = portaudio_root .. "/include"
local openal_include    = openal_root .. "/include"

local buildroot = ""

if PsyProjectExternalLib( "SoLoud", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )

	defines { "MODPLUG_STATIC" }
if (WITH_LIBMODPLUG == 1) then
	defines { "WITH_MODPLUG" }
end		
	
	files 
	{ 
		"./SoLoud/src/audiosource/**.c*",
		"./SoLoud/src/filter/**.c*",
		"./SoLoud/src/core/**.c*",
		"./SoLoud/include/**.h*"
  	}

	includedirs 
	{
		"./SoLoud/src/**",
  		"./SoLoud/include"
	}

if (WITH_OPENAL == 1) then
	defines {"WITH_OPENAL"}
	files
	{
		"./SoLoud/src/backend/openal/**.c*"
	}
	includedirs
	{
		"./SoLoud/include",
		openal_include
	}
end    

if (WITH_OSS == 1) then 
	defines {"WITH_OSS"}
	files
	{
		"./SoLoud/src/backend/oss/**.c*"
	}
	includedirs
	{
		"./SoLoud/include"
	}    
end

if (WITH_PORTAUDIO == 1) then
	defines {"WITH_PORTAUDIO"}
	files
	{
		"./SoLoud/src/backend/portaudio/**.c*"
	}
	includedirs
	{
		"./SoLoud/include",
		portaudio_include
	}
end

if (WITH_SDL == 1) then
	defines { "WITH_SDL" }
	files
	{
		"./SoLoud/src/backend/sdl/**.c*"
	}
	includedirs
	{
		"./SoLoud/include",
		sdl_include
	}
end

if (WITH_SDL_STATIC == 1) then
		defines { "WITH_SDL_STATIC" }
	files {
	  "./SoLoud/src/backend/sdl_static/**.c*"
	  }
	includedirs {
	  "./SoLoud/include",
	  sdl_include
	}
end

if (WITH_SDL2_STATIC == 1) then
		defines { "WITH_SDL2_STATIC" }
	files {
	  "./SoLoud/src/backend/sdl2_static/**.c*"
	  }
	includedirs {
	  "./SoLoud/include",
	  sdl_include
	}
end


if (WITH_WASAPI == 1) then 
	defines { "WITH_WASAPI" }
	files
	{
		"./SoLoud/src/backend/wasapi/**.c*"
	}
	includedirs
	{
		"./SoLoud/include"
	}
end

if (WITH_XAUDIO2 == 1) then
	defines {"WITH_XAUDIO2"}
	files
	{
		"./SoLoud/src/backend/xaudio2/**.c*"
	}
	includedirs
	{
		"./SoLoud/include",
		dxsdk_include
	}
end

if (WITH_WINMM == 1) then
	defines { "WITH_WINMM" }
	files
	{
		"./SoLoud/src/backend/winmm/**.c*"
	}
	includedirs 
	{
		"./SoLoud/include"
	}        
end

if (WITH_NULL == 1) then
	defines { "WITH_NULL" }
	files
	{
		"./SoLoud/src/backend/null/**.c*"
	}
	includedirs 
	{
		"./SoLoud/include"
	}        
end

end 