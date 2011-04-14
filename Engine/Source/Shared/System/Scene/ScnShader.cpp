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
BcBool ScnShader::import( const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();
	const std::string& Type = Object[ "type" ].asString();
	BcFile File;
	
	if( File.open( FileName.c_str(), bcFM_READ ) )
	{
		// Read in whole shader.
		// NOTE: This will actually generate the permutations at some point,
		//       for now we just load in a raw GLSL file and export as default for testing.
		// NOTE: Possibly use cgc to compile them into GLSL/whatever target
		//       to maintain better crossplatform support.
		BcU32 ShaderSize = File.size();
		BcU8* pShader = new BcU8[ ShaderSize ];
		File.read( pShader, ShaderSize );
	
		BcStream HeaderStream;
		
		THeader Header;
		
		// TODO: Better parameter parsing.
		if( Type == "geometry" )
		{
			Header.Type_ = rsST_GEOMETRY;
		}
		else if( Type == "vertex" )
		{
			Header.Type_ = rsST_VERTEX;
		}
		else if( Type == "fragment" )
		{
			Header.Type_ = rsST_FRAGMENT;
		}
		else
		{
			delete [] pShader;
			return BcFalse;
		}
		Header.NoofPermutations_ = 1;

		HeaderStream << Header;
		
		BcStream ShaderStream;

		TShaderHeader ShaderHeader;
		
		ShaderHeader.PermutationFlags_ = scnSPF_DEFAULT;
		
		ShaderStream << ShaderHeader;
		ShaderStream.push( pShader, ShaderSize );
		ShaderStream << BcU8( 0 ); // NULL terminator.
		
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "shader" ), ShaderStream.pData(), ShaderStream.dataSize() );
		pFile_->save();

		delete [] pShader;
		
		return BcTrue;
	}

	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnShader );

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
	// TODO: Lockless list/map.
	return pHeader_ != NULL && ShaderMap_.size() == pHeader_->NoofPermutations_;
}

//////////////////////////////////////////////////////////////////////////
// getShader
RsShader* ScnShader::getShader( BcU32 PermutationFlags )
{
	// Find best matching permutation.
	TShaderMapIterator BestIter = ShaderMap_.find( PermutationFlags );
	
	if( BestIter == ShaderMap_.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( TShaderMapIterator Iter = ShaderMap_.begin(); Iter != ShaderMap_.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (*Iter).first & PermutationFlags );
			if( FlagsSet >= BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one
	if( BestIter != ShaderMap_.end() )
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
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnShader::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
	
		for( BcU32 Idx = 0; Idx < pHeader_->NoofPermutations_; ++Idx )
		{
			pFile_->getChunk( Idx + 1 );
		}
	}
	else if( pChunk->ID_ == BcHash( "shader" ) )
	{
		TShaderHeader* pShaderHeader = (TShaderHeader*)pData;
		void* pShaderData = pShaderHeader + 1;
		BcU32 ShaderSize = pChunk->Size_ - sizeof( TShaderHeader );
		
		if( RsCore::pImpl() != NULL )
		{
			RsShader* pShader = RsCore::pImpl()->createShader( pHeader_->Type_, rsSDT_SOURCE, pShaderData, ShaderSize );
		
			// TODO: Lockless list/map.
			ShaderMap_[ pShaderHeader->PermutationFlags_ ] = pShader;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnShaderProgram );

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnShaderProgram::isReady()
{
	return pProgram_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnShaderProgram::initialise( ScnShaderRef VertexShader, ScnShaderRef FragmentShader )
{
	pProgram_ = NULL;
	VertexShader_ = VertexShader;
	FragmentShader_ = FragmentShader;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnShaderProgram::create()
{
	RsShader* pVertexShader = VertexShader_->getShader( PermutatationFlags_ );
	RsShader* pFragmentShader = FragmentShader_->getShader( PermutatationFlags_ );
	
	pProgram_ = RsCore::pImpl()->createProgram( pVertexShader, pFragmentShader );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnShaderProgram::destroy()
{
	RsCore::pImpl()->destroyResource( pProgram_ );
}

