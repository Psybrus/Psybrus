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

#include "System/Scene/ScnViewComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Renderer/RsCore.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnViewComponent::import( class CsPackageImporter& Importer, const Json::Value& Object )
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
// initialise
void ScnViewComponent::initialise()
{
	// NULL internals.
	//pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnViewComponent::initialise( BcReal X, BcReal Y, BcReal Width, BcReal Height, BcReal Near, BcReal Far, BcReal HorizontalFOV, BcReal VerticalFOV )
{
	Header_.X_ = X;
	Header_.Y_ = Y;
	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Near_ = Near;
	Header_.Far_ = Far;
	Header_.HorizontalFOV_ = HorizontalFOV;
	Header_.VerticalFOV_ = VerticalFOV;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnViewComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Header_.X_ = (BcReal)Object[ "x" ].asDouble();
	Header_.Y_ = (BcReal)Object[ "y" ].asDouble();
	Header_.Width_ = (BcReal)Object[ "width" ].asDouble();
	Header_.Height_ = (BcReal)Object[ "height" ].asDouble();
	Header_.Near_ = (BcReal)Object[ "near" ].asDouble();
	Header_.Far_ = (BcReal)Object[ "far" ].asDouble();
	Header_.HorizontalFOV_ = (BcReal)Object[ "hfov" ].asDouble();
	Header_.VerticalFOV_ = (BcReal)Object[ "vfov" ].asDouble();
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
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnViewComponent::setMaterialParameters( ScnMaterialComponentRef MaterialComponent )
{
	MaterialComponent->setClipTransform( Viewport_.view() * Viewport_.projection() );
	MaterialComponent->setViewTransform( Viewport_.view() );
	MaterialComponent->setEyePosition( InverseViewMatrix_.translation() );	
}

//////////////////////////////////////////////////////////////////////////
// getWorldPosition
void ScnViewComponent::getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far )
{
	// NOTE: Uses last viewport bound.
	Viewport_.unProject( ScreenPosition, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnViewComponent::bind( RsFrame* pFrame, RsRenderSort Sort )
{
	RsContext* pContext = pFrame->getContext();

	// Calculate the viewport.
	const BcReal Width = static_cast< BcReal >( pContext->getWidth() );
	const BcReal Height = static_cast< BcReal >( pContext->getHeight() );
	const BcReal ViewWidth = Header_.Width_ * Width;
	const BcReal ViewHeight = Header_.Height_ * Height;
	const BcReal Aspect = ViewWidth / ViewHeight;

	// Setup the viewport.
	Viewport_.viewport( static_cast< BcU32 >( Header_.X_ * Width ),
	                    static_cast< BcU32 >( Header_.Y_ * Height ),
	                    static_cast< BcU32 >( ViewWidth ),
	                    static_cast< BcU32 >( ViewHeight ),
	                    Header_.Near_,
	                    Header_.Far_ );

	// Setup the view matrix.
	InverseViewMatrix_ = getParentEntity()->getMatrix();
	BcMat4d ViewMatrix( InverseViewMatrix_ );
	ViewMatrix.inverse();	
	Viewport_.view( ViewMatrix );
	
	// Setup the perspective projection.
	BcMat4d ProjectionMatrix;
	if( Header_.HorizontalFOV_ > 0.0f )
	{
		ProjectionMatrix.perspProjectionHorizontal( Header_.HorizontalFOV_, Aspect, Header_.Near_, Header_.Far_ );
	}
	else
	{
		ProjectionMatrix.perspProjectionVertical( Header_.VerticalFOV_, 1.0f / Aspect, Header_.Near_, Header_.Far_ );
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
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnViewComponent::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		// TODO STUFF HERE.
	}
}
