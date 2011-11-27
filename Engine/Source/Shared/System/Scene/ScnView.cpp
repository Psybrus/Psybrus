/**************************************************************************
*
* File:		ScnView.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnView.h"
#include "RsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnView::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	/*
	const std::string& FileName = Object[ "source" ].asString();

	// Add root dependancy.
	DependancyList.push_back( FileName );

	// Load texture from file and create the data for export.
	ImgImage* pImage = Img::load( FileName.c_str() );
	
	if( pImage != NULL )
	{
		BcStream HeaderStream;
		BcStream BodyStream( BcFalse, 1024, ( pImage->width() * pImage->height() * 4 ) );
		
		// TODO: Use parameters to pick a format.
		THeader Header = { pImage->width(), pImage->height(), 1, rsTF_RGBA8 };
		HeaderStream << Header;
		
		// Write body.				
		for( BcU32 Y = 0; Y < pImage->height(); ++Y )
		{
			for( BcU32 X = 0; X < pImage->width(); ++X )
			{
				ImgColour Colour = pImage->getPixel( X, Y );
				
				BodyStream << Colour.R_;
				BodyStream << Colour.G_;
				BodyStream << Colour.B_;
				BodyStream << Colour.A_;
			}
		}
		
		// Delete image.
		delete pImage;
		
		// Add chunks and finish up.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );
				
		//
		return BcTrue;
	}
	*/
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnView );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnView::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnView" )
		.field( "x",			csPVT_UINT,			csPCT_VALUE )
		.field( "y",			csPVT_UINT,			csPCT_VALUE )
		.field( "width",		csPVT_UINT,			csPCT_VALUE )
		.field( "height",		csPVT_UINT,			csPCT_VALUE )
		.field( "fieldofview",	csPVT_REAL,			csPCT_VALUE )
		.field( "near",			csPVT_REAL,			csPCT_VALUE )
		.field( "far",			csPVT_REAL,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnView::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnView::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnView::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnView::isReady()
{
	return pHeader_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnView::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnView::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
				
		// Setup the viewport.
		Viewport_.viewport( pHeader_->X_, pHeader_->Y_, pHeader_->Width_, pHeader_->Height_ );

		// Setup the viewport's projection.
		BcReal Aspect = (BcReal)pHeader_->X_ / (BcReal)pHeader_->Y_;
		BcMat4d ProjectionMatrix;
		ProjectionMatrix.perspProjection( pHeader_->FieldOfView_, Aspect, pHeader_->Near_, pHeader_->Far_ );
	}
}
