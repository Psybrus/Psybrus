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

#include "ScnShader.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnShader::import( const Json::Value& Object, CsDependancyList& DependancyList )
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
			BcFile File;	
			BcStream HeaderStream;
			BcStream VertexShaderStream;
			BcStream FragmentShaderStream;
			BcStream ProgramStream;
			
			THeader Header;
			TShaderHeader ShaderHeader;
			TProgramHeader ProgramHeader;

			Header.NoofVertexShaderPermutations_ = 1;
			Header.NoofFragmentShaderPermutations_ = 1;
			Header.NoofProgramPermutations_ = 1;

			// Load vertex shader.
			if( File.open( VertexShader.asCString(), bcFM_READ ) )
			{	
				// Add dependancy.
				DependancyList.push_back( CsDependancy( VertexShader.asString() ) );

				// Read in whole shader.
				BcU32 ShaderSize = File.size();
				BcU8* pShader = new BcU8[ ShaderSize ];
				File.read( pShader, ShaderSize );
	
				ShaderHeader.PermutationFlags_ = scnSPF_DEFAULT;
		
				VertexShaderStream << ShaderHeader;
				VertexShaderStream.push( pShader, ShaderSize );
				VertexShaderStream << BcU8( 0 ); // NULL terminator.
				delete [] pShader;
				File.close();
			}
			else
			{
				BcPrintf( "ScnShader: No vertex shader called %s\n", VertexShader.asCString() );
			}
	
			// Load fragment shader.
			if( File.open( FragmentShader.asCString(), bcFM_READ ) )
			{
				// Add dependancy.
				DependancyList.push_back( CsDependancy( FragmentShader.asString() ) );

				// Read in whole shader.
				BcU32 ShaderSize = File.size();
				BcU8* pShader = new BcU8[ ShaderSize ];
				File.read( pShader, ShaderSize );
		
				ShaderHeader.PermutationFlags_ = scnSPF_DEFAULT;
		
				FragmentShaderStream << ShaderHeader;
				FragmentShaderStream.push( pShader, ShaderSize );
				FragmentShaderStream << BcU8( 0 ); // NULL terminator.
				delete [] pShader;
				File.close();
			}
			else
			{
				BcPrintf( "ScnShader: No fragment shader called %s\n", VertexShader.asCString() );
			}
			
			// Serialise header.
			HeaderStream << Header;
			
			// Create program.
			ProgramHeader.ProgramPermutationFlags_ = scnSPF_DEFAULT;
			ProgramHeader.VertexShaderPermutationFlags_ = scnSPF_DEFAULT;
			ProgramHeader.FragmentShaderPermutationFlags_ = scnSPF_DEFAULT;
	
			ProgramStream << ProgramHeader;

			// Write out chunks.
			pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
			pFile_->addChunk( BcHash( "vertex" ), VertexShaderStream.pData(), VertexShaderStream.dataSize() );
			pFile_->addChunk( BcHash( "fragment" ), FragmentShaderStream.pData(), FragmentShaderStream.dataSize() );
			pFile_->addChunk( BcHash( "program" ), ProgramStream.pData(), ProgramStream.dataSize() );
		
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

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnShader::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnShader" )
		.field( "shaders",					csPVT_FILE,			csPCT_MAP )
	.endCatagory();
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

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnShader::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnShader::isReady()
{
	BcBool IsReady = pHeader_ != NULL && ProgramMap_.size() == pHeader_->NoofProgramPermutations_;
	
	if( IsReady == BcTrue )
	{
		for( TProgramMapIterator Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
		{
			if( (*Iter).second->hasHandle() == BcFalse )
			{
				IsReady = BcFalse;
				break;
			}
		}
	}
	
	return IsReady;
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
			if( FlagsSet >= BestFlagsSet )
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
			if( FlagsSet >= BestFlagsSet )
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
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnShader::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		getChunk( 0 );
		return;
	}

	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;

		// Grab the rest of the chunks.
		for( BcU32 Idx = 0; Idx < pHeader_->NoofVertexShaderPermutations_; ++Idx )
		{
			getChunk( ++ChunkIdx );
		}

		for( BcU32 Idx = 0; Idx < pHeader_->NoofFragmentShaderPermutations_; ++Idx )
		{
			getChunk( ++ChunkIdx );
		}

		for( BcU32 Idx = 0; Idx < pHeader_->NoofProgramPermutations_; ++Idx )
		{
			getChunk( ++ChunkIdx );
		}
	}
	else if( pChunk->ID_ == BcHash( "vertex" ) )
	{
		TShaderHeader* pShaderHeader = (TShaderHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = pChunk->Size_ - sizeof( TShaderHeader );
		
		RsShader* pShader = RsCore::pImpl()->createShader( rsST_VERTEX, rsSDT_SOURCE, pShaderData, ShaderSize );
		
		// TODO: Lockless list/map.
		VertexShaderMap_[ pShaderHeader->PermutationFlags_ ] = pShader;
	}
	else if( pChunk->ID_ == BcHash( "fragment" ) )
	{
		TShaderHeader* pShaderHeader = (TShaderHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = pChunk->Size_ - sizeof( TShaderHeader );
			
		RsShader* pShader = RsCore::pImpl()->createShader( rsST_FRAGMENT, rsSDT_SOURCE, pShaderData, ShaderSize );
			
		// TODO: Lockless list/map.
		FragmentShaderMap_[ pShaderHeader->PermutationFlags_ ] = pShader;
	}
	else if( pChunk->ID_ == BcHash( "program" ) )
	{
		// Iterate over permutations to generate.
		TProgramHeader* pProgramHeaders = (TProgramHeader*)pData;
		
		for( BcU32 Idx = 0; Idx < pHeader_->NoofProgramPermutations_; ++Idx )
		{
			TProgramHeader* pProgramHeader = &pProgramHeaders[ Idx ];
			
			// Check vertex & fragment shader for closest permutation.
			RsShader* pVertexShader = getShader( pProgramHeader->VertexShaderPermutationFlags_, VertexShaderMap_ );
			RsShader* pFragmentShader = getShader( pProgramHeader->FragmentShaderPermutationFlags_, FragmentShaderMap_ );
			RsProgram* pProgram = RsCore::pImpl()->createProgram( pVertexShader, pFragmentShader );			
			
			ProgramMap_[ pProgramHeader->ProgramPermutationFlags_ ] = pProgram;
		}
	}
}
