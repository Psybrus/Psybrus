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

#include "ScnTextureAtlas.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Img.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTextureAtlas::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& Source = Object[ "source" ];
	
	if( Source.isArray() )
	{
		// Load all source images.
		ImgImageList ImageList;
		for( BcU32 Idx = 0; Idx < Source.size(); ++Idx )
		{
			std::string FileName = Source[ Idx ].asString();  

			// Add as dependancy.
			DependancyList.push_back( FileName );

			// Load image.
			ImgImage* pImage = Img::load( FileName.c_str() );
		
			// Add to list (even if null).
			ImageList.push_back( pImage );
		}

		// If we have images, generate an atlas and export.
		if( ImageList.size() > 0 )
		{
			BcStream HeaderStream;
			BcStream RectsStream;
						
			// Create an atlas of all source textures..
			ImgRectList RectList;
			ImgImage* pAtlasImage = ImgImage::generateAtlas( ImageList, RectList, 1024, 1024 );
			
			// Setup header.
			TAtlasHeader Header = 
			{
				ImageList.size()
			};
			
			HeaderStream << Header;
			
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgRect& Rect = RectList[ Idx ];
				ScnRect OutRect = 
				{
					BcReal( Rect.X_ ) / BcReal( pAtlasImage->width() ),
					BcReal( Rect.Y_ ) / BcReal( pAtlasImage->height() ),
					BcReal( Rect.W_ ) / BcReal( pAtlasImage->width() ),
					BcReal( Rect.H_ ) / BcReal( pAtlasImage->height() )
				};
				
				RectsStream << OutRect;
			}
			
			// Add chunks.
			pFile_->addChunk( BcHash( "atlasheader" ), HeaderStream.pData(), HeaderStream.dataSize() );
			pFile_->addChunk( BcHash( "atlasrects" ), RectsStream.pData(), RectsStream.dataSize() );

			// Create a texture.
			std::string AtlasName = Object[ "name" ].asString() + "_texture_atlas";
			std::string AtlasFileName = std::string( "IntermediateContent/" ) + AtlasName + ".png";
			Img::save( AtlasFileName.c_str(), pAtlasImage );
			
			// Delete all images.
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				delete ImageList[ Idx ];
			}
			ImageList.clear();
			delete pAtlasImage;

				
			// Setup base object, and import.
			Json::Value BaseObject = Object;
			BaseObject[ "source" ] = AtlasFileName;
			
			// Import base texture.
			return Super::import( BaseObject, DependancyList );
		}
	}
		
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTextureAtlas );

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTextureAtlas::getRect( BcU32 Idx )
{
	if( Idx < pAtlasHeader_->NoofTextures_ )
	{
		return pAtlasRects_[ Idx ];
	}

	return ScnTexture::getRect( Idx );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnTextureAtlas::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "atlasheader" ) )
	{
		pAtlasHeader_ = (TAtlasHeader*)pData;
		
		pFile_->getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "atlasrects" ) )
	{
		pAtlasRects_ = (ScnRect*)pData;
		
		pFile_->getChunk( ++ChunkIdx );
	}
	else
	{
		Super::fileChunkReady( ChunkIdx, pChunk, pData );
	}	
}

