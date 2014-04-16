/**************************************************************************
*
* File:		ScnShader.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnShader.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Statics
static ScnShaderPermutationBootstrap GShaderPermutationBootstraps[] = 
{
	{ scnSPF_STATIC_2D | scnSPF_UNLIT,										"Content/Engine/uniforms.glsl", NULL, "Content/Engine/default2dboot.glslv", "Content/Engine/default2dboot.glslf" },
	{ scnSPF_STATIC_3D | scnSPF_UNLIT,										"Content/Engine/uniforms.glsl", NULL, "Content/Engine/default3dboot.glslv", "Content/Engine/default3dboot.glslf" },
	{ scnSPF_SKINNED_3D | scnSPF_UNLIT,										"Content/Engine/uniforms.glsl", NULL, "Content/Engine/default3dskinnedboot.glslv", "Content/Engine/default3dskinnedboot.glslf" },
	{ scnSPF_PARTICLE_3D | scnSPF_UNLIT,									"Content/Engine/uniforms.glsl", NULL, "Content/Engine/particle3dboot.glslv", "Content/Engine/particle3dboot.glslf" },

	{ scnSPF_STATIC_3D | scnSPF_DIFFUSE_LIT,								"Content/Engine/uniforms.glsl", NULL, "Content/Engine/default3ddiffuselitboot.glslv", "Content/Engine/default3ddiffuselitboot.glslf" },
	{ scnSPF_SKINNED_3D | scnSPF_DIFFUSE_LIT,								"Content/Engine/uniforms.glsl", NULL, "Content/Engine/default3dskinneddiffuselitboot.glslv", "Content/Engine/default3dskinneddiffuselitboot.glslf" },
};

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnShader::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	// NOTE: This will only generate 1 permutation. Later on
	//       they will be generated on import based on usage flags.
	const Json::Value& Shaders = Object[ "shaders" ];
	
	// Check we have shaders specified.
	if( Shaders.type() == Json::objectValue )
	{
		const Json::Value& VertexShader = Shaders[ "vertex" ];
		const Json::Value& FragmentShader = Shaders[ "fragment" ];
	
		// Verify we have shaders.
		if( VertexShader.type() == Json::stringValue &&
		    FragmentShader.type() == Json::stringValue )
		{
			BcFile UniformsFile;
			BcFile ShaderFile;
			BcFile BootstrapFile;
			BcStream HeaderStream;
			BcStream VertexShaderStream;
			BcStream FragmentShaderStream;
			BcStream ProgramStream;
			
			ScnShaderHeader Header;
			ScnShaderUnitHeader ShaderHeader;
			ScnShaderProgramHeader ProgramHeader;

			// For now, generate all permutations.
			BcU32 NoofPermutations = sizeof( GShaderPermutationBootstraps ) / sizeof( GShaderPermutationBootstraps[ 0 ] );

			Header.NoofVertexShaderPermutations_ = NoofPermutations;
			Header.NoofFragmentShaderPermutations_ = NoofPermutations;
			Header.NoofProgramPermutations_ = NoofPermutations;

			// Serialise header.
			HeaderStream << Header;
			
			// Write out chunks.
			Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );

			// Load shaders.
			for( BcU32 PermutationIdx = 0; PermutationIdx < NoofPermutations; ++PermutationIdx )
			{
				ScnShaderPermutationBootstrap PermutationBootstrap( GShaderPermutationBootstraps[ PermutationIdx ] );

				if( ShaderFile.open( VertexShader.asCString(), bcFM_READ ) && 
					UniformsFile.open( PermutationBootstrap.SourceUniformIncludeName_, bcFM_READ ) &&
					BootstrapFile.open( PermutationBootstrap.SourceVertexShaderName_, bcFM_READ ) )
				{	
					// Add dependancies.
					Importer.addDependency( VertexShader.asCString() );
					Importer.addDependency( PermutationBootstrap.SourceUniformIncludeName_ );
					Importer.addDependency( PermutationBootstrap.SourceVertexShaderName_ );

					// Setup permutation flags.
					ShaderHeader.PermutationFlags_ = PermutationBootstrap.PermutationFlags_;

					// Read in whole shader.
					BcU8* pUniformsShader = UniformsFile.readAllBytes();
					BcU8* pBootstrapShader = BootstrapFile.readAllBytes();
					BcU8* pShader = ShaderFile.readAllBytes();
			
					// Serialise.
					VertexShaderStream << ShaderHeader;
					VertexShaderStream.push( pUniformsShader, UniformsFile.size() );
					VertexShaderStream.push( pBootstrapShader, BootstrapFile.size() );
					VertexShaderStream.push( pShader, ShaderFile.size() );
					VertexShaderStream << BcU8( 0 ); // NULL terminator.
					delete [] pShader;
					delete [] pBootstrapShader;
					delete [] pUniformsShader;
					ShaderFile.close();
					BootstrapFile.close();
					UniformsFile.close();

					Importer.addChunk( BcHash( "vertex" ), VertexShaderStream.pData(), VertexShaderStream.dataSize() );
					VertexShaderStream.clear();
				}
				else
				{
					BcAssertMsg( BcFalse, "ScnShader: No vertex shader called %s or %s\n", VertexShader.asCString(), PermutationBootstrap.SourceVertexShaderName_ );
				}
				
				// Load fragment shader.
				if( ShaderFile.open( FragmentShader.asCString(), bcFM_READ ) && 
					UniformsFile.open( PermutationBootstrap.SourceUniformIncludeName_, bcFM_READ ) &&
					BootstrapFile.open( PermutationBootstrap.SourceFragmentShaderName_, bcFM_READ ) )
				{
					// Add dependancies.
					Importer.addDependency( FragmentShader.asCString() );
					Importer.addDependency( PermutationBootstrap.SourceUniformIncludeName_ );
					Importer.addDependency( PermutationBootstrap.SourceFragmentShaderName_ );

					// Setup permutation flags.
					ShaderHeader.PermutationFlags_ = PermutationBootstrap.PermutationFlags_;
					
					// Read in whole shader.
					BcU8* pUniformsShader = UniformsFile.readAllBytes();
					BcU8* pBootstrapShader = BootstrapFile.readAllBytes();
					BcU8* pShader = ShaderFile.readAllBytes();
					
					// Serialise.
					FragmentShaderStream << ShaderHeader;
					FragmentShaderStream.push( pUniformsShader, UniformsFile.size() );
					FragmentShaderStream.push( pBootstrapShader, BootstrapFile.size() );
					FragmentShaderStream.push( pShader, ShaderFile.size() );
					FragmentShaderStream << BcU8( 0 ); // NULL terminator.
					delete [] pShader;
					delete [] pBootstrapShader;
					delete [] pUniformsShader;
					ShaderFile.close();
					BootstrapFile.close();
					UniformsFile.close();

					Importer.addChunk( BcHash( "fragment" ), FragmentShaderStream.pData(), FragmentShaderStream.dataSize() );
					FragmentShaderStream.clear();

				}
				else
				{
					BcAssertMsg( BcFalse, "ScnShader: No fragment shader called %s or %s\n", FragmentShader.asCString(), PermutationBootstrap.SourceFragmentShaderName_ );
				}
				
				// Create program.
				ProgramHeader.ProgramPermutationFlags_ = PermutationBootstrap.PermutationFlags_;
				ProgramHeader.VertexShaderPermutationFlags_ = PermutationBootstrap.PermutationFlags_;
				ProgramHeader.FragmentShaderPermutationFlags_ = PermutationBootstrap.PermutationFlags_;
	
				ProgramStream << ProgramHeader;

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
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnShader );

void ScnShader::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "pHeader_",			&ScnShader::pHeader_ ),
		ReField( "VertexShaderMap_",	&ScnShader::VertexShaderMap_ ),
		ReField( "FragmentShaderMap_",	&ScnShader::FragmentShaderMap_ ),
		ReField( "ProgramMap_",			&ScnShader::ProgramMap_ ),
	};
		
	ReRegisterClass< ScnShader, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnShader::initialise()
{
	pHeader_ = NULL;

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnShader::create()
{
	// TODO: Move some of the fileChunk(...) code into here. Will do for now.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnShader::destroy()
{
	for( TShaderMapIterator Iter = VertexShaderMap_.begin(); Iter != VertexShaderMap_.end(); ++Iter )
	{
		RsCore::pImpl()->destroyResource( (*Iter).second );
	}

	for( TShaderMapIterator Iter = FragmentShaderMap_.begin(); Iter != FragmentShaderMap_.end(); ++Iter )
	{
		RsCore::pImpl()->destroyResource( (*Iter).second );
	}

	for( TProgramMapIterator Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
	{
		RsCore::pImpl()->destroyResource( (*Iter).second );
	}

	pHeader_ = NULL;
	VertexShaderMap_.clear();
	FragmentShaderMap_.clear();
	ProgramMap_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getProgram
RsProgram* ScnShader::getProgram( BcU32 PermutationFlags )
{
	// Find best matching permutation.
	TProgramMapIterator BestIter = ProgramMap_.find( PermutationFlags );
	
	if( BestIter == ProgramMap_.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( TProgramMapIterator Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (*Iter).first & PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one.
	if( BestIter != ProgramMap_.end() )
	{
		return (*BestIter).second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getShader
RsShader* ScnShader::getShader( BcU32 PermutationFlags, ScnShader::TShaderMap& ShaderMap )
{
	// Find best matching permutation.
	TShaderMapIterator BestIter = ShaderMap.find( PermutationFlags );
	
	if( BestIter == ShaderMap.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( TShaderMapIterator Iter = ShaderMap.begin(); Iter != ShaderMap.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (*Iter).first & PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one.
	if( BestIter != ShaderMap.end() )
	{
		return (*BestIter).second;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnShader::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnShader::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnShaderHeader*)pData;

		// Grab the rest of the chunks.
		const BcU32 TotalChunks = pHeader_->NoofVertexShaderPermutations_ + pHeader_->NoofFragmentShaderPermutations_ + pHeader_->NoofProgramPermutations_;
		for( BcU32 Idx = 0; Idx < TotalChunks; ++Idx )
		{
			requestChunk( ++ChunkIdx );
		}
	}
	else if( ChunkID == BcHash( "vertex" ) )
	{
		ScnShaderUnitHeader* pShaderHeader = (ScnShaderUnitHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = getChunkSize( ChunkIdx ) - sizeof( ScnShaderUnitHeader );
		
		RsShader* pShader = RsCore::pImpl()->createShader( rsST_VERTEX, rsSDT_SOURCE, pShaderData, ShaderSize );
		
		VertexShaderMap_[ pShaderHeader->PermutationFlags_ ] = pShader;
	}
	else if( ChunkID == BcHash( "fragment" ) )
	{
		ScnShaderUnitHeader* pShaderHeader = (ScnShaderUnitHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = getChunkSize( ChunkIdx ) - sizeof( ScnShaderUnitHeader );
			
		RsShader* pShader = RsCore::pImpl()->createShader( rsST_FRAGMENT, rsSDT_SOURCE, pShaderData, ShaderSize );
			
		FragmentShaderMap_[ pShaderHeader->PermutationFlags_ ] = pShader;
	}
	else if( ChunkID == BcHash( "program" ) )
	{
		// Generate program.
		ScnShaderProgramHeader* pProgramHeader = (ScnShaderProgramHeader*)pData;
			
		// Check vertex & fragment shader for closest permutation.
		RsShader* pVertexShader = getShader( pProgramHeader->VertexShaderPermutationFlags_, VertexShaderMap_ );
		RsShader* pFragmentShader = getShader( pProgramHeader->FragmentShaderPermutationFlags_, FragmentShaderMap_ );
		BcAssertMsg( pVertexShader != NULL, "Vertex shader for permutation %x is invalid in ScnShader %s\n", pProgramHeader->VertexShaderPermutationFlags_, (*getName()).c_str() );
		BcAssertMsg( pFragmentShader != NULL, "Fragment shader for permutation %x is invalid in ScnShader %s\n", pProgramHeader->FragmentShaderPermutationFlags_, (*getName()).c_str() );

		// Create program.
		RsProgram* pProgram = RsCore::pImpl()->createProgram( pVertexShader, pFragmentShader );			
			
		ProgramMap_[ pProgramHeader->ProgramPermutationFlags_ ] = pProgram;

		// Mark ready if we've got all the programs we expect.
		if( ProgramMap_.size() == pHeader_->NoofProgramPermutations_ )
		{
			markCreate();
		}
	}
}
