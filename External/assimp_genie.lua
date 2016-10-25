-- Write out revision file.
--os.execute( "cd assimp && git log -1 --format=%h > git.version" )
--os.execute( "cd assimp && git rev-parse --abbrev-ref HEAD > git.branch" )

--local inVersion = assert( io.open( "assimp/git.version", "r" ) )
local version = "00" -- ( inVersion:read() )
--inVersion:close()

--local inBranch = assert( io.open( "assimp/git.branch", "r" ) )
local branch = "local" -- ( inBranch:read() )
--inBranch:close()

--local revisionFile = assert( io.open( "./assimp/revision.h", "w+" ) )
--revisionFile:write( "#ifndef ASSIMP_REVISION_H_INC\n" )
--revisionFile:write( "#define ASSIMP_REVISION_H_INC\n" )
--revisionFile:write( string.format( "#define GitVersion 0x%s\n", version ) )
--revisionFile:write( string.format( "#define GitBranch \"%s\"\n", branch ) )
--revisionFile:write( "#endif // ASSIMP_REVISION_H_INC\n" )
--revisionFile:close()

if PsyProjectExternalLib( "assimp", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or linux-* or osx-*"
		files { 
			"./assimp/include/**.h", 
			"./assimp/code/**.h",
			"./assimp/code/**.cpp",
		}

		includedirs { 
			"./autogen/assimp",
			"./assimp",
			"./assimp/include",
			"./assimp/code",
			"./assimp/code/BoostWorkaround",
			"./zlib"
		}

		defines { 
			"ASSIMP_BUILD_NO_OWN_ZLIB=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",

			-- Remove importers that we don't want yet to
			-- keep the build lean.
			"ASSIMP_BUILD_NO_X_IMPORTER",
			"ASSIMP_BUILD_NO_MD3_IMPORTER",
			"ASSIMP_BUILD_NO_MDL_IMPORTER",
			"ASSIMP_BUILD_NO_MD2_IMPORTER",
			"ASSIMP_BUILD_NO_HMP_IMPORTER",
			"ASSIMP_BUILD_NO_MDC_IMPORTER",
			"ASSIMP_BUILD_NO_LWO_IMPORTER",
			"ASSIMP_BUILD_NO_NFF_IMPORTER",
			"ASSIMP_BUILD_NO_RAW_IMPORTER",
			"ASSIMP_BUILD_NO_OFF_IMPORTER",
			"ASSIMP_BUILD_NO_AC_IMPORTER",
			"ASSIMP_BUILD_NO_BVH_IMPORTER",
			"ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
			"ASSIMP_BUILD_NO_IRR_IMPORTER",
			"ASSIMP_BUILD_NO_Q3D_IMPORTER",
			"ASSIMP_BUILD_NO_B3D_IMPORTER",
			"ASSIMP_BUILD_NO_COLLADA_IMPORTER",
			"ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
			"ASSIMP_BUILD_NO_CSM_IMPORTER",
			"ASSIMP_BUILD_NO_3D_IMPORTER",
			"ASSIMP_BUILD_NO_3DS_IMPORTER",
			"ASSIMP_BUILD_NO_LWS_IMPORTER",
			"ASSIMP_BUILD_NO_OGRE_IMPORTER",
			"ASSIMP_BUILD_NO_MS3D_IMPORTER",
			"ASSIMP_BUILD_NO_COB_IMPORTER",
			"ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
			"ASSIMP_BUILD_NO_NDO_IMPORTER",
			"ASSIMP_BUILD_NO_IFC_IMPORTER",
			"ASSIMP_BUILD_NO_XGL_IMPORTER",
			"ASSIMP_BUILD_NO_BLEND_IMPORTER",
			"ASSIMP_BUILD_NO_DXF_IMPORTER",
			"ASSIMP_BUILD_NO_STL_IMPORTER",
			"ASSIMP_BUILD_NO_SMD_IMPORTER",
			"ASSIMP_BUILD_NO_PLY_IMPORTER",
			"ASSIMP_BUILD_NO_ASE_IMPORTER"

			-- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
			-- "ASSIMP_BUILD_NO_MD5_IMPORTER",
			-- "ASSIMP_BUILD_NO_FBX_IMPORTER",
		}

		excludes { 
			-- Remove ogre, has compile issues.
			"./assimp/code/Ogre**.*",

			-- Exclude all importers we don't want.
			"./assimp/code/XFile*.*",
			"./assimp/code/MD3*.*",
			"./assimp/code/MDL*.*",
			"./assimp/code/MD2*.*",
			"./assimp/code/HMP*.*",
			"./assimp/code/MDC*.*",
			"./assimp/code/LWO*.*",
			"./assimp/code/NFF*.*",
			"./assimp/code/RAW*.*",
			"./assimp/code/OFF*.*",
			"./assimp/code/AC*.*",
			"./assimp/code/BVH*.*",
			"./assimp/code/IRR*.*",
			"./assimp/code/Q3D*.*",
			"./assimp/code/B3D*.*",
			"./assimp/code/Collada*.*",
			"./assimp/code/Terragen*.*",
			"./assimp/code/CSM*.*",
			"./assimp/code/Unreal*.*",
			"./assimp/code/3DS*.*",
			"./assimp/code/LWS*.*",
			"./assimp/code/Ogr*.*",
			"./assimp/code/Q3*.*",
			"./assimp/code/NDO*.*",
			"./assimp/code/IFC*.*",
			"./assimp/code/XGL*.*",
			"./assimp/code/Blender*.*",
			"./assimp/code/DXF*.*",
			"./assimp/code/STL*.*",
			"./assimp/code/SMD*.*",
			"./assimp/code/PLY*.*",
			"./assimp/code/ASE*.*"
		}

	configuration "vs*"
		pchheader "AssimpPCH.h"
		pchsource "./assimp/code/AssimpPCH.cpp"

	configuration "windows-* or linux-* osx-*"
		links {
			-- External libs.
			"External_assimp_contrib",
		}
end


if PsyProjectExternalLib( "assimp_contrib", "C++" ) then
	kind ( EXTERNAL_PROJECT_KIND )
	configuration "windows-* or linux-* or osx-*"
		files { 
			"./assimp/contrib/clipper/**.cpp",
			"./assimp/contrib/clipper/**.hpp",
			"./assimp/contrib/ConvertUTF/**.c",
			"./assimp/contrib/ConvertUTF/**.h",
			"./assimp/contrib/poly2tri/**.cc",
			"./assimp/contrib/poly2tri/**.h"
		}

		includedirs { 
			"./zlib"
		}

		defines { 
			"ASSIMP_BUILD_NO_OWN_ZLIB=1",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE"
		}
end
