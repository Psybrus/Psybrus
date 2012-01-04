/**************************************************************************
*
* File:		ScnViewComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnViewComponent.h"
#include "RsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnViewComponent::import( const Json::Value& Object, CsDependancyList& DependancyList )
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
DEFINE_RESOURCE( ScnViewComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnViewComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnViewComponent" )
		.field( "x",				csPVT_REAL,			csPCT_VALUE )
		.field( "y",				csPVT_REAL,			csPCT_VALUE )
		.field( "width",			csPVT_REAL,			csPCT_VALUE )
		.field( "height",			csPVT_REAL,			csPCT_VALUE )
		.field( "horizontalfov",	csPVT_REAL,			csPCT_VALUE )
		.field( "verticalfov",		csPVT_REAL,			csPCT_VALUE )
		.field( "near",				csPVT_REAL,			csPCT_VALUE )
		.field( "far",				csPVT_REAL,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnViewComponent::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnViewComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnViewComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnViewComponent::isReady()
{
	return pHeader_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnViewComponent::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnViewComponent::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
				
		// Setup the viewport.
		// TODO: Do this on binding to a context/client.
		Viewport_.viewport( pHeader_->X_, pHeader_->Y_, pHeader_->Width_, pHeader_->Height_, pHeader_->Near_, pHeader_->Far_ );

		// Setup the viewport's projection.
		BcReal Aspect = (BcReal)pHeader_->X_ / (BcReal)pHeader_->Y_;
		BcMat4d ProjectionMatrix;
		if( pHeader_->HorizontalFOV_ > 0.0f )
		{
			ProjectionMatrix.perspProjectionHorizontal( pHeader_->HorizontalFOV_, Aspect, pHeader_->Near_, pHeader_->Far_ );
		}
		else
		{
			ProjectionMatrix.perspProjectionVertical( pHeader_->VerticalFOV_, Aspect, pHeader_->Near_, pHeader_->Far_ );
		}
	}
}
