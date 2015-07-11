/**************************************************************************
*
* File:		ScnShaderImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnShaderImport.h"

#if PSY_IMPORT_PIPELINE

#include "System/SysKernel.h"

#include "Base/BcStream.h"

#define EXCLUDE_PSTDINT
#include <hlslcc.h>
#include <hlsl2glsl.h>
#include <glsl/glsl_optimizer.h>

// Write out shader files to intermediate, and signal game to load the raw files.
// Useful for debugging generated shader files.
#define DEBUG_FILE_WRITE_OUT_FILES		0

#if PLATFORM_WINDOWS
#pragma warning ( disable : 4512 ) // Can't generate assignment operator (for boost)
#endif

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_interface.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/filesystem.hpp>
#include <bitset>

#undef DOMAIN // This is defined somewhere in a core header.

namespace
{
	//////////////////////////////////////////////////////////////////////////
	// Shader permutations.
	static ScnShaderPermutationEntry GPermutationsRenderType[] = 
	{
		{ ScnShaderPermutationFlags::RENDER_FORWARD,			"PERM_RENDER_FORWARD",				"1" },
		{ ScnShaderPermutationFlags::RENDER_DEFERRED,			"PERM_RENDER_DEFERRED",				"1" },
		{ ScnShaderPermutationFlags::RENDER_FORWARD_PLUS,		"PERM_RENDER_FORWARD_PLUS",			"1" },
		{ ScnShaderPermutationFlags::RENDER_POST_PROCESS,		"PERM_RENDER_POST_PROCESS",			"1" },
	};

	static ScnShaderPermutationEntry GPermutationsPassType[] = 
	{
		{ ScnShaderPermutationFlags::PASS_MAIN,					"PERM_PASS_MAIN",					"1" },
		{ ScnShaderPermutationFlags::PASS_SHADOW,				"PERM_PASS_SHADOW",					"1" },
	};

	static ScnShaderPermutationEntry GPermutationsMeshType[] = 
	{
		{ ScnShaderPermutationFlags::MESH_STATIC_2D,			"PERM_MESH_STATIC_2D",				"1" },
		{ ScnShaderPermutationFlags::MESH_STATIC_3D,			"PERM_MESH_STATIC_3D",				"1" },
		{ ScnShaderPermutationFlags::MESH_SKINNED_3D,			"PERM_MESH_SKINNED_3D",				"1" },
		{ ScnShaderPermutationFlags::MESH_PARTICLE_3D,			"PERM_MESH_PARTICLE_3D",			"1" },
		{ ScnShaderPermutationFlags::MESH_INSTANCED_3D,			"PERM_MESH_INSTANCED_3D",			"1" },
	};

	static ScnShaderPermutationEntry GPermutationsLightingType[] = 
	{
		{ ScnShaderPermutationFlags::LIGHTING_NONE,				"PERM_LIGHTING_NONE",				"1" },
		{ ScnShaderPermutationFlags::LIGHTING_DIFFUSE,			"PERM_LIGHTING_DIFFUSE",			"1" },
	};

	static ScnShaderPermutationGroup GPermutationGroups[] =
	{
		ScnShaderPermutationGroup( GPermutationsRenderType ),
		ScnShaderPermutationGroup( GPermutationsPassType ),
		ScnShaderPermutationGroup( GPermutationsMeshType ),
		ScnShaderPermutationGroup( GPermutationsLightingType ),
	};

	static BcU32 GNoofPermutationGroups = ( sizeof( GPermutationGroups ) / sizeof( GPermutationGroups[ 0 ] ) );

	// NOTE: Put these in the order that HLSLCC needs to build them.
	static ScnShaderLevelEntry GShaderLevelEntries[] =
	{
		{ "ps_3_0", RsShaderType::PIXEL, RsShaderCodeType::D3D9_3_0 },
		{ "ps_4_0_level_9_0", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_0 },
		{ "ps_4_0_level_9_1", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_1 },
		{ "ps_4_0_level_9_3", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "ps_4_0", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0 },
		{ "ps_4_1", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_1 },
		{ "ps_5_0", RsShaderType::PIXEL, RsShaderCodeType::D3D11_5_0 },
		
		{ "hs_5_0", RsShaderType::HULL, RsShaderCodeType::D3D11_5_0 },

		{ "ds_5_0", RsShaderType::DOMAIN, RsShaderCodeType::D3D11_5_0 },
		
		{ "gs_4_0", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_0 },
		{ "gs_4_1", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_1 },
		{ "gs_5_0", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_5_0 },
		
		{ "vs_3_0", RsShaderType::VERTEX, RsShaderCodeType::D3D9_3_0 },
		{ "vs_4_0_level_9_0", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_0 },
		{ "vs_4_0_level_9_1", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_1 },
		{ "vs_4_0_level_9_3", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "vs_4_0", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0 },
		{ "vs_4_1", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_1 },
		{ "vs_5_0", RsShaderType::VERTEX, RsShaderCodeType::D3D11_5_0 },
		
		{ "cs_4_0", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_0 },
		{ "cs_4_1", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_1 },
		{ "cs_5_0", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_5_0 },
	};

	static BcU32 GNoofShaderLevelEntries = ( sizeof( GShaderLevelEntries ) / sizeof( GShaderLevelEntries[ 0 ] ) ); 

	static GLLang ConvertShaderCodeTypeToGLLang( RsShaderCodeType CodeType )
	{
		auto BackendType = RsShaderCodeTypeToBackendType( CodeType );
		BcAssert( BackendType == RsShaderBackendType::GLSL ||
		          BackendType == RsShaderBackendType::GLSL_ES );
		BcUnusedVar( BackendType );
		switch( CodeType )
		{
		case RsShaderCodeType::GLSL_140:
			return LANG_140;
		case RsShaderCodeType::GLSL_150:
			return LANG_150;
		case RsShaderCodeType::GLSL_330:
			return LANG_330;
		case RsShaderCodeType::GLSL_400:
			return LANG_400;
		case RsShaderCodeType::GLSL_410:
			return LANG_410;
		case RsShaderCodeType::GLSL_420:
			return LANG_420;
		case RsShaderCodeType::GLSL_430:
			return LANG_430;
		case RsShaderCodeType::GLSL_440:
			return LANG_440;
		case RsShaderCodeType::GLSL_ES_100:
			return LANG_ES_100;
		case RsShaderCodeType::GLSL_ES_300:
			return LANG_ES_300;
		case RsShaderCodeType::GLSL_ES_310:
			return LANG_ES_310;
		default:
			BcBreakpoint;
		}
		return LANG_DEFAULT;
	}

	bool hlsl2glsl_includeFunc( bool isSystem, const char* fname, std::string& output, void* data )
	{
		std::list< std::string > IncludePaths;

#if PLATFORM_WINDOWS
		// Setup include paths.
		IncludePaths.clear();
		IncludePaths.push_back( ".\\" );
		IncludePaths.push_back( "..\\Psybrus\\Dist\\Content\\Engine\\" );
#elif PLATFORM_LINUX || PLATFORM_OSX
		// Setup include paths.
		IncludePaths.clear();
		IncludePaths.push_back( "./" );
		IncludePaths.push_back( "../Psybrus/Dist/Content/Engine/" );
#else
		BcBreakpoint;
#endif
		BcFile IncludeFile;
		for( auto& IncludePath : IncludePaths)
		{
			std::string IncludeFileName = IncludePath + fname;

			if( IncludeFile.open( IncludeFileName.c_str(), bcFM_READ ) )
			{
				// TODO Importer_.addDependency( IncludeFileName.c_str() );
				std::vector< char > FileData( IncludeFile.size() + 1 );
				BcMemZero( FileData.data(), FileData.size() );
				IncludeFile.read( FileData.data(), FileData.size() );
				output = FileData.data();
				return true;
			}
		}
		return false;
	};

}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnShaderImport )
	
void ScnShaderImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnShaderImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Entrypoints_", &ScnShaderImport::Entrypoints_, bcRFF_IMPORTER ),
		new ReField( "ExcludePermutations_", &ScnShaderImport::ExcludePermutations_, bcRFF_IMPORTER ),
		new ReField( "IncludePermutations_", &ScnShaderImport::IncludePermutations_, bcRFF_IMPORTER ),
		new ReField( "CodeTypes_", &ScnShaderImport::CodeTypes_, bcRFF_IMPORTER ),
		new ReField( "BackendTypes_", &ScnShaderImport::BackendTypes_, bcRFF_IMPORTER ),
	};
	ReRegisterClass< ScnShaderImport, Super >( Fields );

	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "NONE", (BcU32)ScnShaderPermutationFlags::NONE ),

			new ReEnumConstant( "RENDER_FORWARD", (BcU32)ScnShaderPermutationFlags::RENDER_FORWARD ),
			new ReEnumConstant( "RENDER_DEFERRED", (BcU32)ScnShaderPermutationFlags::RENDER_DEFERRED ),
			new ReEnumConstant( "RENDER_FORWARD_PLUS", (BcU32)ScnShaderPermutationFlags::RENDER_FORWARD_PLUS ),
			new ReEnumConstant( "RENDER_POST_PROCESS", (BcU32)ScnShaderPermutationFlags::RENDER_POST_PROCESS ),
			new ReEnumConstant( "RENDER_ALL", (BcU32)ScnShaderPermutationFlags::RENDER_ALL ),

			new ReEnumConstant( "PASS_MAIN", (BcU32)ScnShaderPermutationFlags::PASS_MAIN ),
			new ReEnumConstant( "PASS_SHADOW", (BcU32)ScnShaderPermutationFlags::PASS_SHADOW ),
			new ReEnumConstant( "PASS_ALL", (BcU32)ScnShaderPermutationFlags::PASS_ALL ),

			new ReEnumConstant( "MESH_STATIC_2D", (BcU32)ScnShaderPermutationFlags::MESH_STATIC_2D ),
			new ReEnumConstant( "MESH_STATIC_3D", (BcU32)ScnShaderPermutationFlags::MESH_STATIC_3D ),
			new ReEnumConstant( "MESH_SKINNED_3D", (BcU32)ScnShaderPermutationFlags::MESH_SKINNED_3D ),
			new ReEnumConstant( "MESH_PARTICLE_3D", (BcU32)ScnShaderPermutationFlags::MESH_PARTICLE_3D ),
			new ReEnumConstant( "MESH_INSTANCED_3D", (BcU32)ScnShaderPermutationFlags::MESH_INSTANCED_3D ),
			new ReEnumConstant( "MESH_ALL", (BcU32)ScnShaderPermutationFlags::MESH_ALL ),

			new ReEnumConstant( "LIGHTING_NONE", (BcU32)ScnShaderPermutationFlags::LIGHTING_NONE ),
			new ReEnumConstant( "LIGHTING_DIFFUSE", (BcU32)ScnShaderPermutationFlags::LIGHTING_DIFFUSE ),
			new ReEnumConstant( "LIGHTING_ALL", (BcU32)ScnShaderPermutationFlags::LIGHTING_ALL ),
		};
		ReRegisterEnum< ScnShaderPermutationFlags >( EnumConstants );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport() :
	ShaderCompileId_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport( ReNoInit ) :
	ShaderCompileId_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnShaderImport::~ScnShaderImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnShaderImport::import( const Json::Value& )
{
#if PSY_IMPORT_PIPELINE
	if( Source_.empty() )
	{
		PSY_LOG( "ERROR: Missing 'source' field.\n" );
		return BcFalse;
	}

	// Entry points.
	if( Entrypoints_.size() == 0 )
	{
		PSY_LOG( "ERROR: Missing entry points.\n" );
		return BcFalse;
	}

	// Reset errors building.
	GotErrorBuilding_.store( 0 );

	// Reset pending permutations.
	PendingPermutations_.store( 0 );

	// Initialise Hlsl2Glsl.
	Hlsl2Glsl_Initialize();
	
#if PLATFORM_WINDOWS
	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( ".\\" );
	IncludePaths_.push_back( "..\\Psybrus\\Dist\\Content\\Engine\\" );
#elif PLATFORM_LINUX || PLATFORM_OSX
	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( "./" );
	IncludePaths_.push_back( "../Psybrus/Dist/Content/Engine/" );
#endif
	
	// Setup permutations.
	ScnShaderPermutation Permutation;

	// Default to alway use cbuffers.
	Permutation.Defines_[ "PSY_USE_CBUFFER" ] = "1";

	// Cache intermediate path.
	IntermediatePath_ = getIntermediatePath();

	// Read in file.
	BcFile SourceFile;
	if( SourceFile.open( Source_.c_str(), bcFM_READ ) )
	{
		std::vector< char > FileData( SourceFile.size() + 1 );
		BcMemZero( FileData.data(), FileData.size() );
		SourceFile.read( FileData.data(), FileData.size() );
		SourceFileData_ = FileData.data();

		addDependency( Source_.c_str() );
	}

	// Add code type defines.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderCodeType::MAX; ++Idx )
	{
		auto CodeTypeString = RsShaderCodeTypeToString( (RsShaderCodeType)Idx );
		auto Define = boost::str( boost::format( "PSY_CODE_TYPE_%1%" ) % CodeTypeString );
		Permutation.Defines_[ Define ] = boost::str( boost::format( "%1%" ) % Idx );
	}

	// Add backend type defines.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderBackendType::MAX; ++Idx )
	{
		auto BackendTypeString = RsShaderBackendTypeToString( (RsShaderBackendType)Idx );
		auto Define = boost::str( boost::format( "PSY_BACKEND_TYPE_%1%" ) % BackendTypeString );
		Permutation.Defines_[ Define ] = boost::str( boost::format( "%1%" ) % Idx );
	}

	// Generate permutations.
	generatePermutations( 0, GNoofPermutationGroups, GPermutationGroups, Permutation );

	// Sort input types from lowest to highest.
	std::sort( CodeTypes_.begin(), CodeTypes_.end(), []( RsShaderCodeType A, RsShaderCodeType B )
	{
		return A < B;
	});

	// Backend types. If it's empty, default to all.
	if( BackendTypes_.empty() )
	{
		BackendTypes_.push_back( RsShaderBackendType::D3D11 );
		BackendTypes_.push_back( RsShaderBackendType::GLSL );
		BackendTypes_.push_back( RsShaderBackendType::GLSL_ES );
	}

	// Kick off all permutation building jobs.
	BcBool RetVal = BcFalse;
	for( auto& Permutation : Permutations_ )
	{
		for( const auto& InputCodeType : CodeTypes_ )
		{
			// Setup entries for input code type.
			std::vector< ScnShaderLevelEntry > Entries;
			for( auto& ShaderLevelEntry : GShaderLevelEntries )
			{
				const auto& Entry = Entrypoints_[ ShaderLevelEntry.Type_ ];
				if( ShaderLevelEntry.CodeType_ == InputCodeType &&
					!Entry.empty() )
				{
					ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
					Entries.push_back( NewEntry );
				}
			}

			// If we've got valid entries, continue.
			if( Entries.size() > 0 )
			{
				for( const auto& BackendType : BackendTypes_ )
				{
					RsShaderCodeType OutputCodeType = RsConvertCodeTypeToBackendCodeType( InputCodeType, BackendType );

					// If it isn't invalid, add and build.
					if( OutputCodeType != RsShaderCodeType::INVALID )
					{
						// Add output code type.
						if( std::find( OutputCodeTypes_.begin(), OutputCodeTypes_.end(), OutputCodeType ) == OutputCodeTypes_.end() )
						{
							OutputCodeTypes_.push_back( OutputCodeType );
						}

						// Build on a job.
						ScnShaderPermutationJobParams JobParams =
						{
							InputCodeType,
							OutputCodeType,
							Permutation,
							Entries
						};

						++PendingPermutations_;

						SysKernel::pImpl()->pushFunctionJob( 
							0, 
							[ this, JobParams ]()
							{
								if( buildPermutation( JobParams ) == BcFalse )
								{
									;
								}
							} );
					}
				}
			}
		}
	}

	// Wait for permutation building jobs.
	while( PendingPermutations_ > 0 )
	{
		std::this_thread::yield();
	}

	// No errors hit?
	if( GotErrorBuilding_ == 0 )
	{
		RetVal = BcTrue;
	}

	// Export.
	if( RetVal == BcTrue )
	{
		BcStream Stream;
		ScnShaderHeader Header;
		ScnShaderUnitHeader ShaderUnit;
		BcMemZero( &Header, sizeof( Header ) );
		BcMemZero( &ShaderUnit, sizeof( ShaderUnit ) );

		// Export header.
		Header.NoofShaderPermutations_ = static_cast< BcU32 >( BuiltShaderData_.size() );
		Header.NoofProgramPermutations_ = static_cast< BcU32 >( BuiltProgramData_.size() );
		Header.NoofShaderCodeTypes_ = static_cast< BcU32 >( OutputCodeTypes_.size() );
		
		Stream << Header;
		for( auto OutputCodeType : OutputCodeTypes_ )
		{
			Stream << OutputCodeType;
		}

		CsResourceImporter::addChunk( BcHash( "header" ), Stream.pData(), Stream.dataSize() );

		// Export shaders.
		for( auto& ShaderData : BuiltShaderData_ )
		{
			ShaderUnit.ShaderType_ = ShaderData.second.ShaderType_;
			ShaderUnit.ShaderDataType_ = RsShaderDataType::SOURCE;
			ShaderUnit.ShaderCodeType_ = ShaderData.second.CodeType_;
			ShaderUnit.ShaderHash_ = ShaderData.second.Hash_;
			ShaderUnit.PermutationFlags_ = ScnShaderPermutationFlags::NONE;

			Stream.clear();
			Stream.push( &ShaderUnit, sizeof( ShaderUnit ) );
			Stream.push( ShaderData.second.Code_.getData< BcU8* >(), ShaderData.second.Code_.getDataSize() );

			CsResourceImporter::addChunk( BcHash( "shader" ), Stream.pData(), Stream.dataSize() );
		}

		// Export programs.
		BcU32 VertexAttributeIdx = 0;
		for( BcU32 Idx = 0; Idx < BuiltProgramData_.size(); ++Idx )
		{
			auto& ProgramData = BuiltProgramData_[ Idx ];

			Stream.clear();
			Stream.push( &ProgramData, sizeof( ProgramData ) );

			// Only export vertex attributes if it's a shader backend type that requires it.
			if( ProgramData.NoofVertexAttributes_ > 0 )
			{
				auto& VertexAttributes = BuiltVertexAttributes_[ VertexAttributeIdx++ ];
				BcAssert( VertexAttributes.size() > 0 );
				Stream.push( &VertexAttributes[ 0 ], VertexAttributes.size() * sizeof( RsProgramVertexAttribute ) );
			}

			CsResourceImporter::addChunk( BcHash( "program" ), Stream.pData(), Stream.dataSize() );			
		}
	}

	// Shutdown Hlsl2Glsl.
	Hlsl2Glsl_Shutdown();


	return RetVal;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// generatePermutations
void ScnShaderImport::generatePermutations( 
	BcU32 GroupIdx, 
	BcU32 NoofGroups,
	ScnShaderPermutationGroup* PermutationGroups, 
	ScnShaderPermutation Permutation )
{
#if PSY_IMPORT_PIPELINE
	const auto& PermutationGroup = PermutationGroups[ GroupIdx ];

	// Check exclude & include permutations.
	BcU32 GroupExcludeMask = 0;
	BcU32 GroupIncludeMask = 0;
	BcAssert( PermutationGroup.NoofEntries_ < sizeof( GroupExcludeMask ) * 8 );
	BcAssert( PermutationGroup.NoofEntries_ < sizeof( GroupIncludeMask ) * 8 );
	for( BcU32 Idx = 0; Idx < PermutationGroup.NoofEntries_; ++Idx )
	{
		auto PermutationEntry = PermutationGroup.Entries_[ Idx ];
		for( auto ExcludePermutation : ExcludePermutations_ )
		{
			if( PermutationEntry.Flag_ == ExcludePermutation )
			{
				GroupExcludeMask |= 1 << Idx;
				break;
			}
		}

		for( auto IncludePermutation : IncludePermutations_ )
		{
			if( PermutationEntry.Flag_ == IncludePermutation )
			{
				GroupIncludeMask |= 1 << Idx;
				break;
			}
		}
	}

	// Nothing included? Include everything by default.
	if( GroupIncludeMask == 0 )
	{
		GroupIncludeMask = 0xffffffff;
	}

	// Now begin filling permutations in.
	for( BcU32 Idx = 0; Idx < PermutationGroup.NoofEntries_; ++Idx )
	{
		auto PermutationEntry = PermutationGroup.Entries_[ Idx ];

		if( ( ( 1 << Idx ) & GroupIncludeMask ) != 0 &&
			( ( 1 << Idx ) & GroupExcludeMask ) == 0 )
		{
			auto NewPermutation = Permutation; 
			NewPermutation.Flags_ |= PermutationEntry.Flag_;
			NewPermutation.Defines_[ PermutationEntry.Define_ ] = PermutationEntry.Value_;

			if( GroupIdx < ( NoofGroups - 1 ) )
			{
				generatePermutations( GroupIdx + 1, NoofGroups, PermutationGroups, NewPermutation );
			}
			else
			{
				Permutations_.push_back( NewPermutation );
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// buildPermutation
BcBool ScnShaderImport::buildPermutation( ScnShaderPermutationJobParams Params )
{
#if PSY_IMPORT_PIPELINE
	BcBool RetVal = BcTrue;
	BcAssertMsg( RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) == RsShaderBackendType::D3D11, "Expecting D3D11 code input. Need to implement other input backends." );

	// Early out in case any jobs failed previously.
	if( GotErrorBuilding_ != 0 )
	{
		--PendingPermutations_;
		return BcFalse;
	}

	// Add code type and backend types.
	{
		auto Define = boost::str( boost::format( "PSY_CODE_TYPE" ) );
		Params.Permutation_.Defines_[ Define ] = boost::str( boost::format( "%1%" ) % (BcU32)Params.OutputCodeType_ );
	}
	{
		auto Define = boost::str( boost::format( "PSY_BACKEND_TYPE" ) );
		Params.Permutation_.Defines_[ Define ] = boost::str( boost::format( "%1%" ) % (BcU32)RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) );
	}

	// Setup initial header.
	ScnShaderProgramHeader ProgramHeader = {};
	ProgramHeader.ProgramPermutationFlags_ = Params.Permutation_.Flags_;
	ProgramHeader.ShaderFlags_ = ScnShaderPermutationFlags::NONE;
	ProgramHeader.ShaderCodeType_ = Params.OutputCodeType_;
	
	// Cross dependenies needed for GLSL.
    GLSLCrossDependencyData GLSLDependencies;

	// Vertex attributes needed by GLSL.
	std::vector< RsProgramVertexAttribute > VertexAttributes;

    for( auto& Entry : Params.Entries_ )
    {
		BcBinaryData ByteCode;
		std::vector< std::string > ErrorMessages;
		std::string Entrypoint = Entrypoints_[ Entry.Type_ ];
		BcAssert( !Entrypoint.empty() );

		// TODO: Setup shaders to use some kind of pipeline system.
		// HLSL2GLSL path.
		if( RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) == RsShaderBackendType::GLSL_ES )
		{
			std::string OutputShaderCode;
			if( convertHLSL2GLSL( 
					Params, 
					Entry, 
					Entrypoint,
					SourceFileData_, 
					OutputShaderCode,
					VertexAttributes ) )
			{
				ScnShaderBuiltData BuiltShader;
				BuiltShader.ShaderType_ = Entry.Type_;
				BuiltShader.CodeType_ = Params.OutputCodeType_;
				BuiltShader.Code_ = std::move( BcBinaryData( (void*)OutputShaderCode.c_str(), OutputShaderCode.size() + 1, BcTrue ) );
				BuiltShader.Hash_ = generateShaderHash( BuiltShader );

				ProgramHeader.NoofVertexAttributes_ = static_cast< BcU32 >( VertexAttributes.size() );
				ProgramHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShader.Hash_;
#if 0
				// Write out intermediate shader for reference.
				std::string ShaderType;
				switch( Entry.Type_ )
				{
				case RsShaderType::VERTEX:
					ShaderType = "vs";
					break;
				case RsShaderType::HULL:
					ShaderType = "hs";
					break;
				case RsShaderType::DOMAIN:
					ShaderType = "ds";
					break;
				case RsShaderType::GEOMETRY:
					ShaderType = "gs";
					break;
				case RsShaderType::PIXEL:
					ShaderType = "ps";
					break;
				case RsShaderType::COMPUTE:
					ShaderType = "cs";
					break;	
				default:
					BcBreakpoint;
				}

				std::string Path = boost::str( boost::format( "IntermediateContent/%s/%s/%x" ) % RsShaderCodeTypeToString( Params.OutputCodeType_ ) % Name_ % std::bitset< 32 >( (BcU32)ProgramHeader.ProgramPermutationFlags_ ) );
				std::string Filename = boost::str( boost::format( "%s/%s.glsl" ) % Path % ShaderType );

				printf( "ScnShaderImporter: Writing out to %s\n", Path.c_str() );

				{
					std::lock_guard< std::mutex > Lock( BuildingMutex_ );
					boost::filesystem::create_directories( Path );

					BcFile FileOut;
					FileOut.open( Filename.c_str(), bcFM_WRITE );
					FileOut.write( OriginalOutputShaderCode.c_str(), OriginalOutputShaderCode.size() );
					FileOut.close();
				}


				//PSY_LOG( "%s\n", OutputShaderCode.c_str() );
#endif

				std::lock_guard< std::mutex > Lock( BuildingMutex_ );
				auto FoundShader = BuiltShaderData_.find( BuiltShader.Hash_ );
				if( FoundShader != BuiltShaderData_.end() )
				{
					BcAssertMsg( FoundShader->second == BuiltShader, "Hash key collision" );
				}

				BuiltShaderData_[ BuiltShader.Hash_ ] = std::move( BuiltShader );
			}
			else
			{
				PSY_LOG( "Unable to translate." );
				RetVal = BcFalse;
			}
		}
		// HLSLCrossCompiler path.
		else if( compileShader( Source_, Entrypoint, Params.Permutation_.Defines_, IncludePaths_, Entry.Level_, ByteCode, ErrorMessages ) )
		{
			// Shader.
			ScnShaderBuiltData BuiltShader;
			BuiltShader.ShaderType_ = Entry.Type_;
			BuiltShader.CodeType_ = Params.InputCodeType_;
			BuiltShader.Code_ = std::move( ByteCode );
			BuiltShader.Hash_ = generateShaderHash( BuiltShader );
	
			// Headers
			ProgramHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShader.Hash_;

			// Vertex shader attributes.
			if( Entry.Type_ == RsShaderType::VERTEX )
			{
				// Generate vertex attributes.
				VertexAttributes = extractShaderVertexAttributes( BuiltShader.Code_ );
				ProgramHeader.NoofVertexAttributes_ = static_cast< BcU32 >( VertexAttributes.size() );
			}


			if( Params.InputCodeType_ != Params.OutputCodeType_ )
			{
				bool HasGeometry = false;
				bool HasTesselation = false;
				// Patch in geometry shader flag if we have one in the entries list.
				if( std::find_if( Params.Entries_.begin(), Params.Entries_.end(), []( ScnShaderLevelEntry Entry )
					{
						return Entry.Type_ == RsShaderType::GEOMETRY;
					} ) != Params.Entries_.end() )
				{
					HasGeometry = true;
				}

				// Patch in tesselation shader flag if we have one in the entries list.
				if( std::find_if( Params.Entries_.begin(), Params.Entries_.end(), []( ScnShaderLevelEntry Entry )
					{
						return Entry.Type_ == RsShaderType::HULL || Entry.Type_ == RsShaderType::DOMAIN;
					} ) != Params.Entries_.end() )
				{
					HasTesselation = true;
				}
			
				int Flags = HLSLCC_FLAG_GLOBAL_CONSTS_NEVER_IN_UBO | 
				            HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT;

				// GLSL ES needs to not bother with uniform objects.
				if( Params.OutputCodeType_ == RsShaderCodeType::GLSL_ES_100 ||
				    Params.OutputCodeType_ == RsShaderCodeType::GLSL_ES_300 ||
				    Params.OutputCodeType_ == RsShaderCodeType::GLSL_ES_300 )
				{
					Flags = 0;
				}

				// Geometry shader in entries?
				if( HasGeometry )
				{
					Flags |= HLSLCC_FLAG_GS_ENABLED;
				}

				// Tesselation shadrs in entries?
				if( HasTesselation )
				{
					Flags |= HLSLCC_FLAG_TESS_ENABLED;
				}

				// Attempt to convert shaders.
				GLSLShader GLSLResult;
				int GLSLSuccess = TranslateHLSLFromMem( BuiltShader.Code_.getData< const char >(),
					Flags,
					ConvertShaderCodeTypeToGLLang( Params.OutputCodeType_ ),
					nullptr,
					&GLSLDependencies,
					&GLSLResult
					);

				// Check success.
				if( GLSLSuccess )
				{
					// Strip comments out of code for more compact GLSL.
#if DEBUG_FILE_WRITE_OUT_FILES
					std::string GLSLSource = GLSLResult.sourceCode;
#else
					std::string GLSLSource = removeComments( GLSLResult.sourceCode );
#endif

					// Shader.
					BuiltShader.ShaderType_ = Entry.Type_;
					BuiltShader.CodeType_ = Params.OutputCodeType_;
					BuiltShader.Code_ = std::move( BcBinaryData( (void*)GLSLSource.c_str(), GLSLSource.size() + 1, BcTrue ) );
					BuiltShader.Hash_ = generateShaderHash( BuiltShader );

					// Headers
					ProgramHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShader.Hash_;
				
					// Vertex shader attributes.
					if( Entry.Type_ == RsShaderType::VERTEX )
					{
						// Generate vertex attributes.
						VertexAttributes.clear();
						BcAssert( VertexAttributes.size() == 0 );
						VertexAttributes.reserve( GLSLResult.reflection.ui32NumInputSignatures );
						for( BcU32 Idx = 0; Idx < GLSLResult.reflection.ui32NumInputSignatures; ++Idx )
						{
							auto InputSignature = GLSLResult.reflection.psInputSignatures[ Idx ];
							auto VertexAttribute = semanticToVertexAttribute( Idx, InputSignature.SemanticName, InputSignature.ui32SemanticIndex );
							VertexAttributes.push_back( VertexAttribute );
						}

						ProgramHeader.NoofVertexAttributes_ = static_cast< BcU32 >( VertexAttributes.size() );
					}

#if DEBUG_FILE_WRITE_OUT_FILES
					// Write out intermediate shader for reference.
					std::string ShaderType;
					switch( Entry.Type_ )
					{
					case RsShaderType::VERTEX:
						ShaderType = "vs";
						break;
					case RsShaderType::HULL:
						ShaderType = "hs";
						break;
					case RsShaderType::DOMAIN:
						ShaderType = "ds";
						break;
					case RsShaderType::GEOMETRY:
						ShaderType = "gs";
						break;
					case RsShaderType::PIXEL:
						ShaderType = "ps";
						break;
					case RsShaderType::COMPUTE:
						ShaderType = "cs";
						break;	
					default:
						BcBreakpoint;
					}


					std::string Path = boost::str( boost::format( "IntermediateContent/%s/%s/%x" ) % RsShaderCodeTypeToString( Params.OutputCodeType_ ) % Name_ % std::bitset< 32 >( (BcU32)ProgramHeader.ProgramPermutationFlags_ ) );
					std::string Filename = boost::str( boost::format( "%s/%s.glsl" ) % Path % ShaderType );

					printf( "ScnShaderImporter: Writing out to %s\n", Path.c_str() );

					{
						std::lock_guard< std::mutex > Lock( BuildingMutex_ );
						boost::filesystem::create_directories( Path );

						BcFile FileOut;
						FileOut.open( Filename.c_str(), bcFM_WRITE );
						FileOut.write( BuiltShader.Code_.getData< char >(), BuiltShader.Code_.getDataSize() );
						FileOut.close();

						// Overwrite data with indicator to read in from file.
						BcStream FileStream;

						FileStream << ScnShader::LOAD_FROM_FILE_TAG;
						FileStream.push( Filename.c_str(), Filename.size() + 1 );
						BuiltShader.Code_ = std::move( BcBinaryData( (void*)FileStream.pData(), FileStream.dataSize(), BcTrue ) );
						BuiltShader.Hash_ = generateShaderHash( BuiltShader );

						// Headers
						ProgramHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShader.Hash_;
					}
#endif
					// Free GLSL shader.
					FreeGLSLShader( &GLSLResult );
				}
				else
				{
					RetVal = BcFalse;
					throw CsImportException( "Failed to convert to GLSL.", Source_ );
				}
			}
			
			// Push shader into map.
			if( RetVal == BcTrue )
			{
				std::lock_guard< std::mutex > Lock( BuildingMutex_ );
				auto FoundShader = BuiltShaderData_.find( BuiltShader.Hash_ );
				if( FoundShader != BuiltShaderData_.end() )
				{
					BcAssertMsg( FoundShader->second == BuiltShader, "Hash key collision" );
				}

				BuiltShaderData_[ BuiltShader.Hash_ ] = std::move( BuiltShader );
			}
			
			if( ErrorMessages.size() > 0 )
			{
				std::lock_guard< std::mutex > Lock( BuildingMutex_ );
				ErrorMessages_.insert( ErrorMessages_.end(), ErrorMessages.begin(), ErrorMessages.end() );
			}
		}
		else
		{
			RetVal = BcFalse;
			
			if( ErrorMessages.size() > 0 )
			{
				std::lock_guard< std::mutex > Lock( BuildingMutex_ );
				ErrorMessages_.insert( ErrorMessages_.end(), ErrorMessages.begin(), ErrorMessages.end() );
			}
			break;
		}
	}

	// Write out all shaders and programs.
	if( RetVal != BcFalse )
	{
		std::lock_guard< std::mutex > Lock( BuildingMutex_ );
		if( ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::VERTEX ] == 0 ||
			ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::PIXEL ] == 0 )
		{
			PSY_LOG( "No vertex and pixel shaders in program." );
			RetVal = BcFalse;
		}

		BuiltProgramData_.push_back( std::move( ProgramHeader ) );
		if( VertexAttributes.size() > 0 )
		{
			BuiltVertexAttributes_.push_back( VertexAttributes );
		}
	}

	if( RetVal == BcFalse )
	{
		GotErrorBuilding_++;
	}

	// Write out warning/error messages.
	if( ErrorMessages_.size() > 0 )
	{
		std::lock_guard< std::mutex > Lock( BuildingMutex_ );
		std::string Errors;
		for( auto& Error : ErrorMessages_ )
		{
			Errors += Error;
		}

		PSY_LOG( "%s\n%s\n", Source_.c_str(), Errors.c_str() );
		//throw CsImportException( Errors, Filename_ );
	}

	--PendingPermutations_;

	return RetVal;
#else
	return BcFalse;
#endif
}

//////////////////////////////////////////////////////////////////////////
// convertHLSL2GLSL
BcBool ScnShaderImport::convertHLSL2GLSL(
		ScnShaderPermutationJobParams Params,
		ScnShaderLevelEntry LevelEntry,
		const std::string& Entrypoint,
		const std::string& InHLSL, 
		std::string& OutGLSL,
		std::vector< RsProgramVertexAttribute >& OutVertexAttributes )
{
	BcBool RetVal = BcTrue;
#if PSY_IMPORT_PIPELINE
	// Vertex shader attributes.
	EShLanguage Language;

	if( LevelEntry.Type_ == RsShaderType::VERTEX )
	{
		Language = EShLangVertex;
	}
	else if( LevelEntry.Type_ == RsShaderType::PIXEL )
	{
		Language = EShLangFragment;
	}
	else
	{
		BcBreakpoint;
		return BcFalse;
	}

	// Lock as HLSL2GLSL isn't thread safe.
	static std::mutex Mutex;
	std::lock_guard< std::mutex > Lock( Mutex );

	auto CompilerHandle = Hlsl2Glsl_ConstructCompiler( Language );

	Hlsl2Glsl_ParseCallbacks ParseCallbacks = 
	{
		hlsl2glsl_includeFunc,
		this
	};

	// Patch in defines.
	std::string FinalSource;

	// Override cbuffer usage.
	Params.Permutation_.Defines_[ "PSY_USE_CBUFFER" ] = "0";
	for( const auto& Define : Params.Permutation_.Defines_ )
	{
		FinalSource += 
			std::string( "#define " ) +
			Define.first + std::string( " " ) +
			Define.second + std::string( "\n" );
	}

	FinalSource += InHLSL;

	// Parse HLSL.		resultString
	auto ParseRetVal = Hlsl2Glsl_Parse( CompilerHandle, FinalSource.c_str(), ETargetGLSL_ES_100, &ParseCallbacks, 0 );
	BcUnusedVar( ParseRetVal );

	if( ParseRetVal == 0 )
	{
		auto InfoLog = Hlsl2Glsl_GetInfoLog( CompilerHandle );
		ErrorMessages_.push_back( InfoLog );
		RetVal = BcFalse;
	}

	if( RetVal )
	{
		// Translate to GLSL.
		auto TranslateRetVal = Hlsl2Glsl_Translate( CompilerHandle, Entrypoint.c_str(), ETargetGLSL_ES_100, 0 );
		BcUnusedVar( TranslateRetVal );

		if( TranslateRetVal == 0 )
		{
			auto InfoLog = Hlsl2Glsl_GetInfoLog( CompilerHandle );
			ErrorMessages_.push_back( InfoLog );
			RetVal = BcFalse;
		}

		if( RetVal )
		{
			std::string OutputShaderCode = Hlsl2Glsl_GetShader( CompilerHandle );
			std::string OriginalOutputShaderCode = OutputShaderCode;

			// Run through glsl-optimzser.
			auto GlslOptContext = glslopt_initialize( kGlslTargetOpenGLES20 );
			glslopt_set_max_unroll_iterations( GlslOptContext, 32 );

			// Vertex shader attributes.
			glslopt_shader_type GlslOptLanguage;

			if( LevelEntry.Type_ == RsShaderType::VERTEX )
			{
				GlslOptLanguage = kGlslOptShaderVertex;
			}
			else if( LevelEntry.Type_ == RsShaderType::PIXEL )
			{
				GlslOptLanguage = kGlslOptShaderFragment;
			}
			else
			{
				BcBreakpoint;
				return BcFalse;
			}

			auto GlslOptShader = glslopt_optimize( 
				GlslOptContext, 
				GlslOptLanguage,
				OutputShaderCode.c_str(),
				0 );

			if( glslopt_get_status( GlslOptShader ) )
			{
				// Extract shader code.
				OutputShaderCode = glslopt_get_output( GlslOptShader );
			}
			else
			{
				PSY_LOG( "Failed to optimiser GLSL shader:\n%s\n", 
					glslopt_get_log( GlslOptShader ) );
				BcBreakpoint; // TODO: Failed. Why?
			}

			glslopt_cleanup( GlslOptContext );

			// Extract & translate vertex information.
			// Convert from "xlat_attrib__SEMANTIC" into "dcl_Input<x>"
			if( LevelEntry.Type_ == RsShaderType::VERTEX )
			{
				const std::string UsageSemantics[] =
				{
					"xlat_attrib_POSITION",
					"xlat_attrib_BLENDWEIGHTS",
					"xlat_attrib_BLENDINDICES",
					"xlat_attrib_NORMAL",
					"xlat_attrib_PSIZE",
					"xlat_attrib_TEXCOORD",
					"xlat_attrib_TANGENT",
					"xlat_attrib_BINORMAL",
					"xlat_attrib_TESSFACTOR",
					"xlat_attrib_POSITIONT",
					"xlat_attrib_COLOR",
					"xlat_attrib_FOG",
					"xlat_attrib_DEPTH",
					"xlat_attrib_SAMPLE"
				};

				const int MaxUsage = 16;

				OutVertexAttributes.clear();
				RsProgramVertexAttribute Attribute = 
				{
					0, RsVertexUsage::POSITION, 0
				};

				for( int Usage = 0; Usage < (int)RsVertexUsage::MAX; ++Usage )
				{
					for( int Idx = MaxUsage - 1; Idx >= -1; --Idx )
					{
						// Build string to look up.
						std::string Semantic = UsageSemantics[ Usage ];
						if( Idx != -1 )
						{
							Semantic = Semantic + std::to_string( Idx );
						}

						// Does the code have this semantic?
						if( OutputShaderCode.find( Semantic ) != std::string::npos )
						{
							std::string NewSemantic = std::string( "dcl_Input" ) + std::to_string( Attribute.Channel_ );
							boost::replace_all( OutputShaderCode, Semantic, NewSemantic );	

							Attribute.Usage_ = (RsVertexUsage)Usage;
							Attribute.UsageIdx_ = Idx == 0 -1 ? 0 : (BcU32)Idx;
							OutVertexAttributes.push_back( Attribute );

							// Advance channel.
							Attribute.Channel_++;
						}
					}
				}
			}

			// Destruct compiler
			Hlsl2Glsl_DestructCompiler( CompilerHandle );

			// Finalise shader.
			OutGLSL = removeComments( OutputShaderCode );
			RetVal = BcTrue;
		}
	}
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// generateShaderHash
BcU32 ScnShaderImport::generateShaderHash( const ScnShaderBuiltData& Data )
{
	BcU32 Hash = 0;
	Hash = BcHash::GenerateCRC32( Hash, &Data.ShaderType_, sizeof( Data.ShaderType_ ) );
	Hash = BcHash::GenerateCRC32( Hash, &Data.CodeType_, sizeof( Data.CodeType_ ) );
	Hash = BcHash::GenerateCRC32( Hash, Data.Code_.getData< BcU8* >(), Data.Code_.getDataSize() );
	return Hash;
}

//////////////////////////////////////////////////////////////////////////
// removeComments
std::string ScnShaderImport::removeComments( std::string Input )
{
#if PSY_IMPORT_PIPELINE
	std::string Output;
	typedef boost::wave::cpplexer::lex_token<> token_type;
	typedef boost::wave::cpplexer::lex_iterator<token_type> lexer_type;
	typedef token_type::position_type position_type;

	position_type pos;

	lexer_type Iter = lexer_type(Input.begin(), Input.end(), pos, 
		boost::wave::language_support( 
			boost::wave::support_cpp | boost::wave::support_option_long_long ) );
	lexer_type End = lexer_type();

	for ( ; Iter != End; ++Iter )
	{
		if ( *Iter != boost::wave::T_CCOMMENT && 
			 *Iter != boost::wave::T_CPPCOMMENT )
		{
			Output += std::string( Iter->get_value().begin(), Iter->get_value().end() );
		}
	}
	return std::move( Output );
#else
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////
// semanticToVertexChannel
RsProgramVertexAttribute ScnShaderImport::semanticToVertexAttribute( BcU32 Channel, const std::string& Name, BcU32 Index )
{
	RsProgramVertexAttribute VertexAttribute;

	VertexAttribute.Channel_ = Channel;
	VertexAttribute.Usage_ = RsVertexUsage::INVALID;
	VertexAttribute.UsageIdx_ = Index;

	if( Name == "POSITION" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITION;
	}
	else if( Name == "SV_POSITION" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITION;
	}
	else if( Name == "BLENDWEIGHTS" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BLENDWEIGHTS;
	}
	else if( Name == "BLENDINDICES" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BLENDINDICES;
	}
	else if( Name == "NORMAL" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::NORMAL;
	}
	else if( Name == "PSIZE" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::PSIZE;
	}
	else if( Name == "TEXCOORD" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TEXCOORD;
	}
	else if( Name == "TANGENT" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TANGENT;
	}
	else if( Name == "BINORMAL" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BINORMAL;
	}
	else if( Name == "TESSFACTOR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TESSFACTOR;
	}
	else if( Name == "POSITIONT" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITIONT;
	}
	else if( Name == "COLOR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::COLOUR;
	}
	else if( Name == "FOG" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::FOG;
	}
	else if( Name == "DEPTH" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::DEPTH;
	}
	else if( Name == "SAMPLE" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::SAMPLE;
	}
	else
	{
		BcBreakpoint;
	}

	return VertexAttribute;
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void ScnShaderImport::addDependency( const BcChar* Dependency )
{
	CsResourceImporter::addDependency( Dependency );
}
