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

#include "System/SysKernel.h"

#ifdef PSY_SERVER

#include "Base/BcStream.h"

#define EXCLUDE_PSTDINT
#include <hlslcc.h>

// Write out shader files to intermediate, and signal game to load the raw files.
// Useful for debugging generated shader files.
#define DEBUG_FILE_WRITE_OUT_FILES		0

#include <boost/format.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_interface.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/filesystem.hpp>
#include <bitset>

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
		{ "ps_4_0_level_9_3",				"pixel",		RsShaderType::FRAGMENT, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "ps_4_0",							"pixel",		RsShaderType::FRAGMENT, RsShaderCodeType::D3D11_4_0 },
		{ "ps_4_1",							"pixel",		RsShaderType::FRAGMENT, RsShaderCodeType::D3D11_4_1 },
		{ "ps_5_0",							"pixel",		RsShaderType::FRAGMENT, RsShaderCodeType::D3D11_5_0 },
		
		{ "hs_5_0",							"hull",			RsShaderType::TESSELATION_CONTROL, RsShaderCodeType::D3D11_5_0 },

		{ "ds_5_0",							"domain",		RsShaderType::TESSELATION_EVALUATION, RsShaderCodeType::D3D11_5_0 },
		
		{ "gs_4_0",							"geometry",		RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_0 },
		{ "gs_4_1",							"geometry",		RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_1 },
		{ "gs_5_0",							"geometry",		RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_5_0 },
		
		{ "vs_4_0_level_9_3",				"vertex",		RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "vs_4_0",							"vertex",		RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0 },
		{ "vs_4_1",							"vertex",		RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_1 },
		{ "vs_5_0",							"vertex",		RsShaderType::VERTEX, RsShaderCodeType::D3D11_5_0 },
		
		{ "cs_4_0",							"compute",		RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_0 },
		{ "cs_4_1",							"compute",		RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_1 },
		{ "cs_5_0",							"compute",		RsShaderType::COMPUTE, RsShaderCodeType::D3D11_5_0 },
	};

	static BcU32 GNoofShaderLevelEntries = ( sizeof( GShaderLevelEntries ) / sizeof( GShaderLevelEntries[ 0 ] ) ); 

	static GLLang ConvertShaderCodeTypeToGLLang( RsShaderCodeType CodeType )
	{
		auto BackendType = RsShaderCodeTypeToBackendType( CodeType );
		BcAssert( BackendType == RsShaderBackendType::GLSL ||
		          BackendType == RsShaderBackendType::GLSL_ES );
		switch( CodeType )
		{
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
		}
		return LANG_DEFAULT;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport( class CsPackageImporter& Importer ):
	Importer_( Importer )
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
BcBool ScnShaderImport::import( const Json::Value& Object )
{
	const Json::Value& Shader = Object[ "shader" ];
	if( Shader.type() == Json::nullValue )
	{
		BcPrintf( "WARNING: Shader has not been updated to use latest shader importer.\n" );
		return BcFalse;
	}

	auto PsybrusSDKRoot = std::getenv( "PSYBRUS_SDK" );
	BcAssertMsg( PsybrusSDKRoot != nullptr, "Environment variable PSYBRUS_SDK is not set. Have you ran setup.py to configure this?" );

	// Grab shader entries to build.
	const Json::Value& InputEntries = Object[ "entries" ];

	// File name.
	if( Shader.type() != Json::stringValue )
	{
		return BcFalse;
	}

	ResourceName_ = Object[ "name" ].asString();
	Filename_ = Shader.asCString();

	// Entries.
	if( InputEntries.type() != Json::objectValue )
	{
		return BcFalse;
	}

	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( ".\\" );
	IncludePaths_.push_back( std::string( PsybrusSDKRoot ) + "\\Dist\\Content\\Engine\\" );
	
	// Setup permutations.
	ScnShaderPermutation Permutation;
	Permutation.Defines_[ "PSY_USE_CBUFFER" ] = "1";

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

	// Add all input code types, but default to the lowest D3D11 level
	// if we don't specify. We could autodetect later depending on 
	// shader entry points specified.
	InputCodeTypes_.push_back( RsShaderCodeType::D3D11_4_0_LEVEL_9_3 );
	const Json::Value& InputCodeTypes = Object[ "codetypes" ];
	if( InputCodeTypes.type() == Json::arrayValue )
	{
		if( InputCodeTypes.size() > 0 )
		{
			InputCodeTypes_.clear();
			for( auto InputCodeType : InputCodeTypes )
			{
				RsShaderCodeType CodeType = RsStringToShaderCodeType( InputCodeType.asString() );

				// Only support D3D11 cross compilation for now.
				BcAssert( RsShaderCodeTypeToBackendType( CodeType ) == RsShaderBackendType::D3D11 );
				InputCodeTypes_.push_back( CodeType );
			}
		}
	}

	// Sort input types from lowest to highest.
	std::sort( InputCodeTypes_.begin(), InputCodeTypes_.end(), []( RsShaderCodeType A, RsShaderCodeType B )
	{
		return A < B;
	});

	// Backend types.
	BackendTypes_.clear();
	BackendTypes_.push_back( RsShaderBackendType::D3D11 );
	BackendTypes_.push_back( RsShaderBackendType::GLSL );

	// Kick off all permutation building jobs.
	BcBool RetVal = BcTrue;
	for( auto& Permutation : Permutations_ )
	{
		for( const auto& InputCodeType : InputCodeTypes_ )
		{
			// Setup entries for input code type.
			std::vector< ScnShaderLevelEntry > Entries;
			for( auto& ShaderLevelEntry : GShaderLevelEntries )
			{
				auto& Entry = InputEntries[ ShaderLevelEntry.Entry_ ];
				if( Entry.type() == Json::stringValue && 
					ShaderLevelEntry.CodeType_ == InputCodeType )
				{
					ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
					NewEntry.Entry_ = Entry.asCString();
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

						typedef BcDelegate< BcBool(*)( ScnShaderPermutationJobParams ) > BuildPermutationDelegate;
						BuildPermutationDelegate JobDelegate =
							BuildPermutationDelegate::bind< ScnShaderImport, &ScnShaderImport::buildPermutation >( this );
						SysKernel::pImpl()->pushDelegateJob( 0, JobDelegate, JobParams );
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
		Header.NoofShaderPermutations_ = BuiltShaderData_.size();
		Header.NoofProgramPermutations_ = BuiltProgramData_.size();
		Header.NoofShaderCodeTypes_ = OutputCodeTypes_.size();
		
		Stream << Header;
		for( auto OutputCodeType : OutputCodeTypes_ )
		{
			Stream << OutputCodeType;
		}

		Importer_.addChunk( BcHash( "header" ), Stream.pData(), Stream.dataSize() );

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

			Importer_.addChunk( BcHash( "shader" ), Stream.pData(), Stream.dataSize() );
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

			Importer_.addChunk( BcHash( "program" ), Stream.pData(), Stream.dataSize() );			
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// generatePermutations
void ScnShaderImport::generatePermutations( 
	BcU32 GroupIdx, 
	BcU32 NoofGroups,
	ScnShaderPermutationGroup* PermutationGroups, 
	ScnShaderPermutation Permutation )
{
	const auto& PermutationGroup = PermutationGroups[ GroupIdx ];

	for( BcU32 Idx = 0; Idx < PermutationGroup.NoofEntries_; ++Idx )
	{
		auto PermutationEntry = PermutationGroup.Entries_[ Idx ];
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

//////////////////////////////////////////////////////////////////////////
// buildPermutation
BcBool ScnShaderImport::buildPermutation( ScnShaderPermutationJobParams Params )
{
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

		if( compileShader( Filename_, Entry.Entry_, Params.Permutation_.Defines_, IncludePaths_, Entry.Level_, ByteCode, ErrorMessages ) )
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
				ProgramHeader.NoofVertexAttributes_ = VertexAttributes.size();
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
						return Entry.Type_ == RsShaderType::TESSELATION_CONTROL || Entry.Type_ == RsShaderType::TESSELATION_EVALUATION;
					} ) != Params.Entries_.end() )
				{
					HasTesselation = true;
				}
			
				int Flags = HLSLCC_FLAG_GLOBAL_CONSTS_NEVER_IN_UBO | 
				            HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT;

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
				int GLSLSuccess = TranslateHLSLFromMem( ByteCode.getData< const char >(),
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

						ProgramHeader.NoofVertexAttributes_ = VertexAttributes.size();
					}

					// Write out intermediate shader for reference.
					std::string ShaderType;
					switch( Entry.Type_ )
					{
					case RsShaderType::VERTEX:
						ShaderType = "vs";
						break;
					case RsShaderType::TESSELATION_CONTROL:
						ShaderType = "hs";
						break;
					case RsShaderType::TESSELATION_EVALUATION:
						ShaderType = "ds";
						break;
					case RsShaderType::GEOMETRY:
						ShaderType = "gs";
						break;
					case RsShaderType::FRAGMENT:
						ShaderType = "fs";
						break;
					case RsShaderType::COMPUTE:
						ShaderType = "cs";
						break;	
					}

#if DEBUG_FILE_WRITE_OUT_FILES
					std::string Path = boost::str( boost::format( "IntermediateContent/%s/%s/%x" ) % RsShaderCodeTypeToString( Params.OutputCodeType_ ) % ResourceName_ % std::bitset< 32 >( (BcU32)ProgramHeader.ProgramPermutationFlags_ ) );
					std::string Filename = boost::str( boost::format( "%s/%s.glsl" ) % Path % ShaderType );
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
					throw CsImportException( "Failed to convert to GLSL.", Filename_ );
				}
			}
			
			// Push shader into map.
			if( RetVal == BcTrue )
			{
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

		BcPrintf( "%s\n%s\n", Filename_.c_str(), Errors.c_str() );
		//throw CsImportException( Errors, Filename_ );
	}

	--PendingPermutations_;

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
		if ( *Iter != boost::wave::T_CCOMMENT && *Iter != boost::wave::T_CPPCOMMENT )
		{
			Output += std::string( Iter->get_value().begin(), Iter->get_value().end() );
		}
	}
	return std::move( Output );
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

#endif
