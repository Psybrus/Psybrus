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

#ifdef PSY_SERVER

#include "Base/BcStream.h"

#define EXCLUDE_PSTDINT

#include <hlslcc.h>

#include <boost/format.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_interface.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/filesystem.hpp>

namespace
{
	//////////////////////////////////////////////////////////////////////////
	// Legacy boot strap shader generation.
	struct ScnShaderPermutationBootstrap
	{
		BcU32							PermutationFlags_;
		const BcChar*					SourceUniformIncludeName_;
		const BcChar*					SourceVertexShaderName_;
		const BcChar*					SourceFragmentShaderName_;
		const BcChar*					SourceGeometryShaderName_;
	};

	static ScnShaderPermutationBootstrap GShaderPermutationBootstraps[] = 
	{
		{ scnSPF_MESH_STATIC_2D | scnSPF_LIGHTING_NONE,									"Content/Engine/uniforms.glsl", "Content/Engine/default2dboot.glslv", "Content/Engine/default2dboot.glslf", "" },
		{ scnSPF_MESH_STATIC_3D | scnSPF_LIGHTING_NONE,									"Content/Engine/uniforms.glsl", "Content/Engine/default3dboot.glslv", "Content/Engine/default3dboot.glslf", "" },
		{ scnSPF_MESH_SKINNED_3D | scnSPF_LIGHTING_NONE,								"Content/Engine/uniforms.glsl", "Content/Engine/default3dskinnedboot.glslv", "Content/Engine/default3dskinnedboot.glslf", "Content/Engine/default3dskinnedboot.glslg" },
		{ scnSPF_MESH_PARTICLE_3D | scnSPF_LIGHTING_NONE,								"Content/Engine/uniforms.glsl", "Content/Engine/particle3dboot.glslv", "Content/Engine/particle3dboot.glslf", "" },

		{ scnSPF_MESH_STATIC_3D | scnSPF_LIGHTING_DIFFUSE,								"Content/Engine/uniforms.glsl", "Content/Engine/default3ddiffuselitboot.glslv", "Content/Engine/default3ddiffuselitboot.glslf", "" },
		{ scnSPF_MESH_SKINNED_3D | scnSPF_LIGHTING_DIFFUSE,								"Content/Engine/uniforms.glsl", "Content/Engine/default3dskinneddiffuselitboot.glslv", "Content/Engine/default3dskinneddiffuselitboot.glslf", "" },
	};

	//////////////////////////////////////////////////////////////////////////
	// New permutations.
	static ScnShaderPermutationEntry GPermutationsRenderType[] = 
	{
		{ scnSPF_RENDER_FORWARD,			"PERM_RENDER_FORWARD",			"1" },
		{ scnSPF_RENDER_DEFERRED,			"PERM_RENDER_DEFERRED",			"1" },
		{ scnSPF_RENDER_FORWARD_PLUS,		"PERM_RENDER_FORWARD_PLUS",		"1" },
		{ scnSPF_RENDER_POST_PROCESS,		"PERM_RENDER_POST_PROCESS",		"1" },
	};

	static ScnShaderPermutationEntry GPermutationsMeshType[] = 
	{
		{ scnSPF_MESH_STATIC_2D,			"PERM_MESH_STATIC_2D",			"1" },
		{ scnSPF_MESH_STATIC_3D,			"PERM_MESH_STATIC_3D",			"1" },
		{ scnSPF_MESH_SKINNED_3D,			"PERM_MESH_SKINNED_3D",			"1" },
		{ scnSPF_MESH_PARTICLE_3D,			"PERM_MESH_PARTICLE_3D",		"1" },
		{ scnSPF_MESH_INSTANCED_3D,			"PERM_MESH_INSTANCED_3D",		"1" },
	};

	static ScnShaderPermutationEntry GPermutationsLightingType[] = 
	{
		{ scnSPF_LIGHTING_NONE,				"PERM_LIGHTING_NONE",			"1" },
		{ scnSPF_LIGHTING_DIFFUSE,			"PERM_LIGHTING_DIFFUSE",		"1" },
	};

	static ScnShaderPermutationGroup GPermutationGroups[] =
	{
		ScnShaderPermutationGroup( GPermutationsRenderType ),
		ScnShaderPermutationGroup( GPermutationsMeshType ),
		ScnShaderPermutationGroup( GPermutationsLightingType ),
	};

	static BcU32 GNoofPermutationGroups = ( sizeof( GPermutationGroups ) / sizeof( GPermutationGroups[ 0 ] ) );

	// NOTE: Put these in the order that HLSLCC needs to build them.
	static ScnShaderLevelEntry GShaderLevelEntries[] =
	{
		{ "ps_4_0",							"",					RsShaderType::FRAGMENT },
		{ "ps_4_0_level_9_1",				"",					RsShaderType::FRAGMENT },
		{ "ps_4_0_level_9_3",				"",					RsShaderType::FRAGMENT },
		{ "ps_4_1",							"",					RsShaderType::FRAGMENT },
		{ "ps_5_0",							"",					RsShaderType::FRAGMENT },

		{ "hs_5_0",							"",					RsShaderType::TESSELATION_CONTROL },
		{ "ds_5_0",							"",					RsShaderType::TESSELATION_EVALUATION },

		{ "gs_4_0",							"",					RsShaderType::GEOMETRY },
		{ "gs_4_1",							"",					RsShaderType::GEOMETRY },
		{ "gs_5_0",							"",					RsShaderType::GEOMETRY },

		{ "vs_4_0",							"",					RsShaderType::VERTEX },
		{ "vs_4_0_level_9_1",				"",					RsShaderType::VERTEX },
		{ "vs_4_0_level_9_3",				"",					RsShaderType::VERTEX },
		{ "vs_4_1",							"",					RsShaderType::VERTEX },
		{ "vs_5_0",							"",					RsShaderType::VERTEX },

		{ "cs_4_0",							"",					RsShaderType::COMPUTE },
		{ "cs_4_1",							"",					RsShaderType::COMPUTE },
		{ "cs_5_0",							"",					RsShaderType::COMPUTE },
	};

	static BcU32 GNoofShaderLevelEntries = ( sizeof( GShaderLevelEntries ) / sizeof( GShaderLevelEntries[ 0 ] ) ); 
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnShaderImport::import( class CsPackageImporter& Importer, const Json::Value& Object )
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
	const Json::Value& Entries = Object[ "entries" ];

	// File name.
	if( Shader.type() != Json::stringValue )
	{
		return BcFalse;
	}

	Filename_ = Shader.asCString();

	// Entries.
	if( Entries.type() != Json::objectValue )
	{
		return BcFalse;
	}

	// TODO: Check if there are any missing.
	for( auto& ShaderLevelEntry : GShaderLevelEntries )
	{
		auto& Entry = Entries[ ShaderLevelEntry.Level_ ];
		if( Entry.type() == Json::stringValue )
		{
			ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
			NewEntry.Entry_ = Entry.asCString();
			Entries_.push_back( NewEntry );
		}
	}

	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( ".\\" );
	IncludePaths_.push_back( std::string( PsybrusSDKRoot ) + "\\Dist\\Content\\Engine\\" );
	
	// Generate permutations.
	ScnShaderPermutation Permutation;
	Permutation.Defines_[ "PSY_USE_CBUFFER" ] = "1";
	generatePermutations( 0, GNoofPermutationGroups, GPermutationGroups, Permutation );

	// Iterate over permutations to build.
	// TODO: Parallelise.
	BcBool RetVal = BcTrue;
	for( auto& Permutation : Permutations_ )
	{
		BcBool RetVal = buildPermutation( Importer, Permutation );

		if( RetVal == BcFalse )
		{
			RetVal = BcFalse;
			break;
		}
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

		Importer.addChunk( BcHash( "header" ), &Header, sizeof( Header ) );

		// Export shaders.
		for( auto& ShaderData : BuiltShaderData_ )
		{
			ShaderUnit.ShaderType_ = ShaderData.second.ShaderType_;
			ShaderUnit.ShaderDataType_ = RsShaderDataType::SOURCE;
			ShaderUnit.ShaderCodeType_ = ShaderData.second.CodeType_;
			ShaderUnit.ShaderHash_ = ShaderData.second.Hash_;
			ShaderUnit.PermutationFlags_ = 0;

			Stream.clear();
			Stream.push( &ShaderUnit, sizeof( ShaderUnit ) );
			Stream.push( ShaderData.second.Code_.getData< BcU8* >(), ShaderData.second.Code_.getDataSize() );

			Importer.addChunk( BcHash( "shader" ), Stream.pData(), Stream.dataSize() );
		}

		// Export programs.
		BcAssert( BuiltProgramData_.size() == BuiltVertexAttributes_.size() );
		for( BcU32 Idx = 0; Idx < BuiltProgramData_.size(); ++Idx )
		{
			auto& ProgramData = BuiltProgramData_[ Idx ];
			auto& VertexAttributes = BuiltVertexAttributes_[ Idx ];

			Stream.clear();
			Stream.push( &ProgramData, sizeof( ProgramData ) );
			BcAssert( VertexAttributes.size() > 0 );
			Stream.push( &VertexAttributes[ 0 ], VertexAttributes.size() * sizeof( RsProgramVertexAttribute ) );
	
			Importer.addChunk( BcHash( "program" ), Stream.pData(), Stream.dataSize() );			
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// generatePermutations
void ScnShaderImport::generatePermutations( BcU32 GroupIdx, 
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
BcBool ScnShaderImport::buildPermutation( class CsPackageImporter& Importer, const ScnShaderPermutation& Permutation )
{
	BcBool RetVal = BcTrue;

	// Cross dependenies needed for GLSL.
	GLSLCrossDependencyData GLSLDependencies;

	ScnShaderProgramHeader D3D11Header;
	ScnShaderProgramHeader GLSLHeader;
	BcMemZero( &D3D11Header, sizeof( D3D11Header ) );
	BcMemZero( &GLSLHeader, sizeof( GLSLHeader ) );

	D3D11Header.ProgramPermutationFlags_ = Permutation.Flags_;
	D3D11Header.ShaderFlags_ = 0;
	D3D11Header.ShaderCodeType_ = scnSCT_D3D11_5_1;
	GLSLHeader.ProgramPermutationFlags_ = Permutation.Flags_;
	GLSLHeader.ShaderFlags_ = 0;
	GLSLHeader.ShaderCodeType_ = scnSCT_GLSL_430;

	bool HasGeometry = false;
	bool HasTesselation = false;
	// Patch in geometry shader flag if we have one in the entries list.
	if( std::find_if( Entries_.begin(), Entries_.end(), []( ScnShaderLevelEntry Entry )
		{
			return Entry.Type_ == RsShaderType::GEOMETRY;
		} ) != Entries_.end() )
	{
		HasGeometry = true;
	}

	// Patch in tesselation shader flag if we have one in the entries list.
	if( std::find_if( Entries_.begin(), Entries_.end(), []( ScnShaderLevelEntry Entry )
		{
			return Entry.Type_ == RsShaderType::TESSELATION_CONTROL || Entry.Type_ == RsShaderType::TESSELATION_EVALUATION;
		} ) != Entries_.end() )
	{
		HasTesselation = true;
	}

	std::vector< RsProgramVertexAttribute > VertexAttributes;
	
	for( auto& Entry : Entries_ )
	{
		int Flags = HLSLCC_FLAG_GLOBAL_CONSTS_NEVER_IN_UBO | 
	                HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT;

		// Geometry shader in entries?
		if( HasGeometry )
		{
			if( Entry.Type_ == RsShaderType::VERTEX )
			{
				Flags |= HLSLCC_FLAG_GS_ENABLED;
			}
		}

		// Tesselation shadrs in entries?
		if( HasTesselation )
		{
			if( Entry.Type_ == RsShaderType::TESSELATION_CONTROL ||
				Entry.Type_ == RsShaderType::TESSELATION_EVALUATION )
			{
				Flags |= HLSLCC_FLAG_TESS_ENABLED;
			}
		}

		BcBinaryData ByteCode;
		if( compileShader( Filename_, Entry.Entry_, Permutation.Defines_, IncludePaths_, Entry.Level_, ByteCode, ErrorMessages_ ) )
		{
			// Shader.
			ScnShaderBuiltData D3D11Shader;
			D3D11Shader.ShaderType_ = Entry.Type_;
			D3D11Shader.CodeType_ = scnSCT_D3D11_5_1;
			D3D11Shader.Code_ = std::move( ByteCode );
			D3D11Shader.Hash_ = BcHash( D3D11Shader.Code_.getData< const BcU8 >(), D3D11Shader.Code_.getDataSize() );

			// Attempt to convert shaders.
			GLSLShader GLSLResult;
			int GLSLSuccess = TranslateHLSLFromMem( ByteCode.getData< const char >(),
				Flags,
				LANG_430,
				nullptr,
				&GLSLDependencies,
				&GLSLResult
				);

			// Check success.
			if( GLSLSuccess )
			{
				// Strip comments out of code for more compact GLSL.
				std::string GLSLSource = removeComments( GLSLResult.sourceCode );

				// Shader.
				ScnShaderBuiltData GLSLShader;
				GLSLShader.ShaderType_ = Entry.Type_;
				GLSLShader.CodeType_ = scnSCT_GLSL_430;
				GLSLShader.Code_ = std::move( BcBinaryData( (void*)GLSLSource.c_str(), GLSLSource.size() + 1, BcTrue ) );
				GLSLShader.Hash_ = BcHash( GLSLShader.Code_.getData< const BcU8 >(), GLSLShader.Code_.getDataSize() );

				// Push shaders into map.
				auto FoundD3D11Shader = BuiltShaderData_.find( D3D11Shader.Hash_ );
				if( FoundD3D11Shader != BuiltShaderData_.end() )
				{
					BcAssertMsg( FoundD3D11Shader->second == D3D11Shader, "Hash key collision" );
				}

				auto FoundGLSLShader = BuiltShaderData_.find( GLSLShader.Hash_ );
				if( FoundGLSLShader != BuiltShaderData_.end() )
				{
					BcAssertMsg( FoundGLSLShader->second == GLSLShader, "Hash key collision" );
				}

				BuiltShaderData_[ D3D11Shader.Hash_ ] = std::move( D3D11Shader );
				BuiltShaderData_[ GLSLShader.Hash_ ] = std::move( GLSLShader );

				// Headers
				D3D11Header.ShaderHashes_[ (BcU32)Entry.Type_ ] = D3D11Shader.Hash_;
				GLSLHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = GLSLShader.Hash_;
				
				// Vertex shader attributes.
				if( Entry.Type_ == RsShaderType::VERTEX )
				{
					// Generate vertex attributes.
					BcAssert( VertexAttributes.size() == 0 );
					VertexAttributes.reserve( GLSLResult.reflection.ui32NumInputSignatures );
					for( BcU32 Idx = 0; Idx < GLSLResult.reflection.ui32NumInputSignatures; ++Idx )
					{
						auto InputSignature = GLSLResult.reflection.psInputSignatures[ Idx ];
						auto VertexAttribute = semanticToVertexAttribute( Idx, InputSignature.SemanticName, InputSignature.ui32SemanticIndex );
						VertexAttributes.push_back( VertexAttribute );
					}

					D3D11Header.NoofVertexAttributes_ = VertexAttributes.size();
					GLSLHeader.NoofVertexAttributes_ = VertexAttributes.size();
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
				std::string Path = boost::str( boost::format( "IntermediateContent/%s/%x" ) % Filename_ % GLSLHeader.ProgramPermutationFlags_ );
				std::string Filename = boost::str( boost::format( "%s/%s.glsl" ) % Path % ShaderType );
				boost::filesystem::create_directories( Path );

				BcFile FileOut;
				FileOut.open( Filename.c_str(), bcFM_WRITE );
				FileOut.write( GLSLShader.Code_.getData< char >(), GLSLShader.Code_.getDataSize() );
				FileOut.close();

				// Free GLSL shader.
				FreeGLSLShader( &GLSLResult );
			}
			else
			{
				RetVal = BcFalse;
				throw CsImportException( "Failed to convert to GLSL.", Filename_ );
			}
		}
		else
		{
			RetVal = BcFalse;
			break;
		}
	}

	// Write out all shaders and programs.
	if( RetVal != BcFalse )
	{
		BcAssert( VertexAttributes.size() > 0 );

		BuiltProgramData_.push_back( std::move( D3D11Header ) );
		BuiltProgramData_.push_back( std::move( GLSLHeader ) );
		BuiltVertexAttributes_.push_back( VertexAttributes );
		BuiltVertexAttributes_.push_back( VertexAttributes );
	}

	// Write out warning/error messages.
	if( ErrorMessages_.size() > 0 )
	{
		std::string Errors;
		for( auto& Error : ErrorMessages_ )
		{
			Errors += Error;
		}

		throw CsImportException( Errors, Filename_ );
	}

	return RetVal;
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
	else if( Name == "BINORMAL" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BINORMAL;
	}
	else if( Name == "TESSFACTOR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TESSFACTOR;
	}
	else if( Name == "POISITIONT" )
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

	return VertexAttribute;
}

#endif
