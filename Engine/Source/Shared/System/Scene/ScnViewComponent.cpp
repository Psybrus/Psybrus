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
#include "ScnEntity.h"

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
void ScnViewComponent::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnViewComponent::initialise( BcReal X, BcReal Y, BcReal Width, BcReal Height, BcReal Near, BcReal Far, BcReal HorizontalFOV, BcReal VerticalFOV )
{
	// Temporary solution.
	pHeader_ = &TempImportHeaderHack_;
	
	pHeader_->X_ = X;
	pHeader_->Y_ = Y;
	pHeader_->Width_ = Width;
	pHeader_->Height_ = Height;
	pHeader_->Near_ = Near;
	pHeader_->Far_ = Far;
	pHeader_->HorizontalFOV_ = HorizontalFOV;
	pHeader_->VerticalFOV_ = VerticalFOV;
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
// setMaterialParameters
void ScnViewComponent::setMaterialParameters( ScnMaterialComponentRef MaterialComponent )
{
	BcU32 ClipTransform = MaterialComponent->findParameter( "uClipTransform" );
	MaterialComponent->setParameter( ClipTransform, Viewport_.view() * Viewport_.projection() );
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnViewComponent::bind( RsFrame* pFrame, RsRenderSort Sort )
{
	RsContext* pContext = pFrame->getContext();

	// Calculate the viewport.
	const BcReal Width = static_cast< BcReal >( pContext->getWidth() );
	const BcReal Height = static_cast< BcReal >( pContext->getHeight() );
	const BcReal ViewWidth = pHeader_->Width_ * Width;
	const BcReal ViewHeight = pHeader_->Height_ * Height;
	const BcReal Aspect = ViewWidth / ViewHeight;

	// Setup the viewport.
	Viewport_.viewport( static_cast< BcU32 >( pHeader_->X_ * Width ),
	                    static_cast< BcU32 >( pHeader_->Y_ * Height ),
	                    static_cast< BcU32 >( ViewWidth ),
	                    static_cast< BcU32 >( ViewHeight ),
	                    pHeader_->Near_,
	                    pHeader_->Far_ );

	// Setup the view matrix.
	BcMat4d ViewMatrix;
	getParentEntity()->getTransform().getInvertedMatrix( ViewMatrix );
	Viewport_.view( ViewMatrix );
	
	// Setup the perspective projection.
	BcMat4d ProjectionMatrix;
	if( pHeader_->HorizontalFOV_ > 0.0f )
	{
		ProjectionMatrix.perspProjectionHorizontal( pHeader_->HorizontalFOV_, Aspect, pHeader_->Near_, pHeader_->Far_ );
	}
	else
	{
		ProjectionMatrix.perspProjectionVertical( pHeader_->VerticalFOV_, Aspect, pHeader_->Near_, pHeader_->Far_ );
	}
	Viewport_.projection( ProjectionMatrix );

	// Set render target.
	if( RenderTarget_.isValid() )
	{
		RenderTarget_->bind( pFrame );
	}
	else
	{
		pFrame->setRenderTarget( NULL );
	}
	
	// Set viewport.
	pFrame->setViewport( Viewport_ );
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
				
		// TODO STUFF HERE.
	}
}
