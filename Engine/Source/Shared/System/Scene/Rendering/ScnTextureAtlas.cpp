/**************************************************************************
*
* File:		ScnTextureAtlas.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnTextureAtlas.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTextureImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTextureAtlas );

void ScnTextureAtlas::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pAtlasHeader_",		&ScnTextureAtlas::pAtlasHeader_ ),
	};
		
	auto& Class = ReRegisterClass< ScnTextureAtlas, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnTextureImport::StaticGetClass(), 0 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureAtlas::ScnTextureAtlas()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
ScnTextureAtlas::~ScnTextureAtlas()
{

}

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTextureAtlas::getRect( BcU32 Idx )
{
	if( Idx < pAtlasHeader_->NoofTextures_ )
	{
		return pAtlasRects_[ Idx ].Rect_;
	}

	return ScnTexture::getRect( Idx );
}

//////////////////////////////////////////////////////////////////////////
// noofRects
//virtual
BcU32 ScnTextureAtlas::noofRects()
{
	return pAtlasHeader_->NoofTextures_;
}


//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnTextureAtlas::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "atlasheader" ) )
	{
		pAtlasHeader_ = (ScnTextureAtlasHeader*)pData;
		
		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "atlasrects" ) )
	{
		pAtlasRects_ = (ScnTextureAtlasRect*)pData;
		
		requestChunk( ++ChunkIdx, &Header_ );
	}
	else
	{
		Super::fileChunkReady( ChunkIdx, ChunkID, pData );
	}	
}

