boostRoot = os.getenv("BOOST_ROOT") or "C:/Boost"
boostInclude = boostRoot .. "/include/boost-1_55"


-- Write out revision file.
os.execute( "cd assimp && git log -1 --format=%h > git.version" )
os.execute( "cd assimp && git rev-parse --abbrev-ref HEAD > git.branch" )

local inVersion = assert( io.open( "assimp/git.version", "r" ) )
local version = ( inVersion:read() )
inVersion:close()

local inBranch = assert( io.open( "assimp/git.branch", "r" ) )
local branch = ( inBranch:read() )
inBranch:close()


local revisionFile = assert( io.open( "./assimp/revision.h", "w+" ) )
revisionFile:write( "#ifndef ASSIMP_REVISION_H_INC\n" )
revisionFile:write( "#define ASSIMP_REVISION_H_INC\n" )
revisionFile:write( string.format( "#define GitVersion 0x%s\n", version ) )
revisionFile:write( string.format( "#define GitBranch \"%s\"\n", branch ) )
revisionFile:write( "#endif // ASSIMP_REVISION_H_INC\n" )
revisionFile:close()

project "External_assimp"
	kind "StaticLib"
	language "C++"
	files { 
		"./assimp/include/**.h", 
		"./assimp/code/**.h",
		"./assimp/code/**.cpp" 
	}
	
	defines { 
		"ASSIMP_BUILD_NO_OWN_ZLIB=1",
		"_SCL_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_DEPRECATE"
	}

	excludes { 
		-- Remove boost workaround.
		"./assimp/code/BoostWorkaround/**.*" ,

		-- Remove importers we don't want.
		"./assimp/code/3DS*.*", 
		"./assimp/code/AC*.*", 
		"./assimp/code/ASE*.*", 
		"./assimp/code/B3D*.*", 
		"./assimp/code/IFC*.*", 
		"./assimp/code/IRR*.*", 
		"./assimp/code/irr*.*", 
		"./assimp/code/HMP*.*", 
		"./assimp/code/LW*.*", 
		"./assimp/code/Ogre*.*", 
		"./assimp/code/Ply*.*", 
		"./assimp/code/NDO*.*",
		"./assimp/code/OFF*.*",
		"./assimp/code/STEP*.*",
		"./assimp/code/XFile*.*",
		"./assimp/code/XGL*.*",
	}

	includedirs { 
		"./assimp",
		"./assimp/include",
		"./zlib",
		boostInclude
	}

