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
		{ "ps_4_0",							"",					rsST_FRAGMENT },
		{ "ps_4_0_level_9_1",				"",					rsST_FRAGMENT },
		{ "ps_4_0_level_9_3",				"",					rsST_FRAGMENT },
		{ "ps_4_1",							"",					rsST_FRAGMENT },
		{ "ps_5_0",							"",					rsST_FRAGMENT },

		{ "hs_5_0",							"",					rsST_TESSELATION_CONTROL },
		{ "ds_5_0",							"",					rsST_TESSELATION_EVALUATION },

		{ "vs_4_0",							"",					rsST_VERTEX },
		{ "vs_4_0_level_9_1",				"",					rsST_VERTEX },
		{ "vs_4_0_level_9_3",				"",					rsST_VERTEX },
		{ "vs_4_1",							"",					rsST_VERTEX },
		{ "vs_5_0",							"",					rsST_VERTEX },

		{ "gs_4_0",							"",					rsST_GEOMETRY },
		{ "gs_4_1",							"",					rsST_GEOMETRY },
		{ "gs_5_0",							"",					rsST_GEOMETRY },

		{ "cs_4_0",							"",					rsST_COMPUTE },
		{ "cs_4_1",							"",					rsST_COMPUTE },
		{ "cs_5_0",							"",					rsST_COMPUTE },
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
		return legacyImport( Importer, Object );
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
			ShaderUnit.ShaderDataType_ = rsSDT_SOURCE;
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
// legacyImport
BcBool ScnShaderImport::legacyImport( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const Json::Value& Shaders = Object[ "shaders" ];
	
	// Check we have shaders specified.
	if( Shaders.type() == Json::objectValue )
	{
		const Json::Value& VertexShader = Shaders[ "vertex" ];
		const Json::Value& FragmentShader = Shaders[ "fragment" ];
		const Json::Value& GeometryShader = Shaders[ "geometry" ];
	
		// Verify we have shaders.
		if( VertexShader.type() == Json::stringValue &&
		    FragmentShader.type() == Json::stringValue )
		{
			BcStream HeaderStream;
			BcStream ProgramStream;
			
			ScnShaderHeader Header;
			ScnShaderProgramHeader ProgramHeader;
			BcMemZero( &Header, sizeof( Header ) );
			BcMemZero( &ProgramHeader, sizeof( ProgramHeader ) );

			// For now, generate all permutations.
			BcU32 NoofPermutations = sizeof( GShaderPermutationBootstraps ) / sizeof( GShaderPermutationBootstraps[ 0 ] );

			Header.NoofShaderPermutations_ = NoofPermutations * 2;
			Header.NoofProgramPermutations_ = NoofPermutations;

			// Serialise header.
			HeaderStream << Header;
			
			// Write out chunks.
			Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );

			// Load shaders.
			for( BcU32 PermutationIdx = 0; PermutationIdx < NoofPermutations; ++PermutationIdx )
			{
				ScnShaderPermutationBootstrap PermutationBootstrap( GShaderPermutationBootstraps[ PermutationIdx ] );

				auto LoadShader = [ this, &Importer, &PermutationBootstrap ]( eRsShaderType Type,
				                                                              const std::string& Shader,
				                                                              const std::string& UniformInclude, 
				                                                              const std::string& Bootstrap )
				{
					ScnShaderUnitHeader ShaderHeader;
					BcMemZero( &ShaderHeader, sizeof( ShaderHeader ) );

					BcFile UniformsFile;
					BcFile ShaderFile;
					BcFile BootstrapFile;
					BcStream Stream;
					if( ShaderFile.open( Shader.c_str(), bcFM_READ ) )
					{	
						// Read in whole shader.
						BcU8* pShader = ShaderFile.readAllBytes();
						BcU8* pUniformsShader = nullptr;
						BcU8* pBootstrapShader = nullptr;

						// Load include if need be.
						if( UniformInclude.length() > 0 && UniformsFile.open( UniformInclude.c_str(), bcFM_READ ) )
						{
							pUniformsShader = UniformsFile.readAllBytes();
						}

						// Load in bootstrap if need be.
						if( Bootstrap.length() > 0 && BootstrapFile.open( Bootstrap.c_str(), bcFM_READ ) )
						{
							pBootstrapShader = BootstrapFile.readAllBytes();
						}

						// Add dependancies.
						Importer.addDependency( Shader.c_str() );
						Importer.addDependency( UniformInclude.c_str() );
						Importer.addDependency( Bootstrap.c_str() );

						// Setup permutation flags.
						ShaderHeader.ShaderType_ = Type;
						ShaderHeader.ShaderDataType_ = rsSDT_SOURCE;
						ShaderHeader.ShaderCodeType_ = scnSCT_GLSL_150;
						ShaderHeader.PermutationFlags_ = PermutationBootstrap.PermutationFlags_;
			
						// Serialise.
						Stream << ShaderHeader;
						if( pUniformsShader != nullptr )
						{
							Stream.push( pUniformsShader, UniformsFile.size() );
						}

						if( pBootstrapShader != nullptr )
						{
							Stream.push( pBootstrapShader, BootstrapFile.size() );
						}

						Stream.push( pShader, ShaderFile.size() );
						Stream << BcU8( 0 ); // NULL terminator.
						BcMemFree( pShader );
						BcMemFree( pBootstrapShader );
						BcMemFree( pUniformsShader );

						ShaderFile.close();
						BootstrapFile.close();
						UniformsFile.close();

						Importer.addChunk( BcHash( "shader" ), Stream.pData(), Stream.dataSize() );
						Stream.clear();

						return true;
					}
					else
					{
						BcAssertMsg( BcFalse, "ScnShader: No shader of type %u called %s or %s\n", Type, Shader.c_str(), Bootstrap.c_str() );
					}

					return false;
				};
				bool VertexLoaded = false;
				bool FragmentLoaded = false;
				bool GeometryLoaded = false;
				
				if( VertexShader.type() == Json::stringValue )
				{
					VertexLoaded = LoadShader( rsST_VERTEX, VertexShader.asCString(), PermutationBootstrap.SourceUniformIncludeName_, PermutationBootstrap.SourceVertexShaderName_ );
					BcAssertMsg( VertexLoaded, "Failed to load vertex shader" );
				}
				
				if( FragmentShader.type() == Json::stringValue )
				{
					FragmentLoaded = LoadShader( rsST_FRAGMENT, FragmentShader.asCString(), PermutationBootstrap.SourceUniformIncludeName_, PermutationBootstrap.SourceFragmentShaderName_ );
					BcAssertMsg( FragmentLoaded, "Failed to load fragment shader" );
				}
				
				if( GeometryShader.type() == Json::stringValue )
				{
					GeometryLoaded = LoadShader( rsST_GEOMETRY, GeometryShader.asCString(), "", PermutationBootstrap.SourceGeometryShaderName_ );
					BcAssertMsg( GeometryLoaded, "Failed to load geometry shader" );
				}

				// Create program.
				ProgramHeader.ProgramPermutationFlags_ = PermutationBootstrap.PermutationFlags_;
				ProgramHeader.ShaderFlags_ = 
					( VertexLoaded ? ( 1 << rsST_VERTEX ) : 0 ) |
					( FragmentLoaded ? ( 1 << rsST_FRAGMENT ) : 0 ) |
					( GeometryLoaded ? ( 1 << rsST_GEOMETRY ) : 0 );
				ProgramHeader.NoofVertexAttributes_ = rsVC_MAX;
				ProgramHeader.ShaderCodeType_ = scnSCT_GLSL_150;
	
				ProgramStream << ProgramHeader;

				// Export vertex attributes.
				RsProgramVertexAttribute VertexAttributes[] = 
				{
					{ Importer.addString( "aPosition" ),		rsVC_POSITION },
					{ Importer.addString( "aNormal" ),			rsVC_NORMAL },
					{ Importer.addString( "aTangent" ),			rsVC_TANGENT },
					{ Importer.addString( "aTexCoord0" ),		rsVC_TEXCOORD0 },
					{ Importer.addString( "aTexCoord1" ),		rsVC_TEXCOORD1 },
					{ Importer.addString( "aTexCoord2" ),		rsVC_TEXCOORD2 },
					{ Importer.addString( "aTexCoord3" ),		rsVC_TEXCOORD3 },
					{ Importer.addString( "aTexCoord4" ),		rsVC_TEXCOORD4 },
					{ Importer.addString( "aTexCoord5" ),		rsVC_TEXCOORD5 },
					{ Importer.addString( "aTexCoord6" ),		rsVC_TEXCOORD6 },
					{ Importer.addString( "aTexCoord7" ),		rsVC_TEXCOORD7 },
					{ Importer.addString( "aSkinIndices" ),		rsVC_SKIN_INDICES },
					{ Importer.addString( "aSkinWeights" ),		rsVC_SKIN_WEIGHTS },
					{ Importer.addString( "aColour" ),			rsVC_COLOUR },
				};

				ProgramStream << VertexAttributes;

				Importer.addChunk( BcHash( "program" ), ProgramStream.pData(), ProgramStream.dataSize() );
				ProgramStream.clear();
			}
		
			return BcTrue;
		}
		else
		{
			BcPrintf( "ScnShader: Not all shaders specified.\n" );
		}
	}
	else
	{
		BcPrintf( "ScnShader: Shaders not listed.\n" );
	}
	
	return BcFalse;
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
	BcBool RetVal = BcFalse;

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
	GLSLHeader.ShaderCodeType_ = scnSCT_GLSL_150;
	
	std::vector< RsProgramVertexAttribute > VertexAttributes;
	
	for( auto& Entry : Entries_ )
	{
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
				HLSLCC_FLAG_GLOBAL_CONSTS_NEVER_IN_UBO | HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT,
				LANG_150,
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
				GLSLShader.CodeType_ = scnSCT_GLSL_150;
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
				D3D11Header.ShaderHashes_[ Entry.Type_ ] = D3D11Shader.Hash_;
				GLSLHeader.ShaderHashes_[ Entry.Type_ ] = GLSLShader.Hash_;
				
				// Vertex shader attributes.
				if( Entry.Type_ == rsST_VERTEX )
				{
					// Generate vertex attributes.
					BcAssert( VertexAttributes.size() == 0 );
					VertexAttributes.reserve( GLSLResult.reflection.ui32NumInputSignatures );
					for( BcU32 Idx = 0; Idx < GLSLResult.reflection.ui32NumInputSignatures; ++Idx )
					{
						auto InputSignature = GLSLResult.reflection.psInputSignatures[ Idx ];
						auto VertexChannel = semanticToVertexChannel( InputSignature.SemanticName, InputSignature.ui32SemanticIndex );
						BcAssert( VertexChannel != rcVC_INVALID );

						RsProgramVertexAttribute VertexAttribute;
						VertexAttribute.AttributeName_ = Importer.addString( boost::str( boost::format( "decl_Input%s" ) % Idx ).c_str() );
						VertexAttribute.Channel_ = VertexChannel;
						VertexAttributes.push_back( VertexAttribute );
					}

					D3D11Header.NoofVertexAttributes_ = VertexAttributes.size();
					GLSLHeader.NoofVertexAttributes_ = VertexAttributes.size();
				}

				// Free GLSL shader.
				FreeGLSLShader( &GLSLResult );
			}
			RetVal = BcTrue;
		}
		else
		{
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
		BcPrintf( "ScnShaderImport: Errors:\n" );
		for( auto& Error : ErrorMessages_ )
		{
			BcPrintf( Error.c_str() );
		}
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
eRsVertexChannel ScnShaderImport::semanticToVertexChannel( const std::string& Name, BcU32 Index )
{
	if( Name == "POSITION" && Index == 0 )
	{
		return rsVC_POSITION;
	}
	else if( Name == "NORMAL" && Index == 0 )
	{
		return rsVC_NORMAL;
	}
	else if( Name == "TANGENT" && Index == 0 )
	{
		return rsVC_TANGENT;
	}
	else if( Name == "TEXCOORD" && Index == 0 )
	{
		return rsVC_TEXCOORD0;
	}
	else if( Name == "TEXCOORD" && Index == 1 )
	{
		return rsVC_TEXCOORD1;
	}
	else if( Name == "TEXCOORD" && Index == 2 )
	{
		return rsVC_TEXCOORD2;
	}
	else if( Name == "TEXCOORD" && Index == 3 )
	{
		return rsVC_TEXCOORD3;
	}
	else if( Name == "TEXCOORD" && Index == 4 )
	{
		return rsVC_TEXCOORD4;
	}
	else if( Name == "TEXCOORD" && Index == 5 )
	{
		return rsVC_TEXCOORD5;
	}
	else if( Name == "TEXCOORD" && Index == 6 )
	{
		return rsVC_TEXCOORD6;
	}
	else if( Name == "TEXCOORD" && Index == 7 )
	{
		return rsVC_TEXCOORD7;
	}
	else if( Name == "BLENDINDICES" && Index == 0 )
	{
		return rsVC_SKIN_INDICES;
	}
	else if( Name == "BLENDWEIGHTS" && Index == 0 )
	{
		return rsVC_SKIN_WEIGHTS;
	}
	else if( Name == "COLOR" && Index == 0 )
	{
		return rsVC_COLOUR;
	}

	return rcVC_INVALID;
}

#endif
