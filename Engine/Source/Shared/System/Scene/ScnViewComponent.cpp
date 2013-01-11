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

#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Scene/ScnViewComponent.h"
#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnEntity.h"

#include "System/Scene/ScnRenderingVisitor.h"

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

BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnViewComponent )
	BCREFLECTION_MEMBER( BcF32,							X_,								bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							Y_,								bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							Width_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							Height_,						bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							Near_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							Far_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							HorizontalFOV_,					bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,							VerticalFOV_,					bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcU32,								RenderMask_,					bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( ScnRenderTarget,					RenderTarget_,					bcRFF_REFERENCE ),
	BCREFLECTION_MEMBER( BcMat4d,							InverseViewMatrix_,				bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( RsViewport,						Viewport_,						bcRFF_DEFAULT | bcRFF_TRANSIENT )
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnViewComponent::initialise()
{
	Super::initialise();

	// NULL internals.
	//pHeader_ = NULL;

	setRenderMask( 1 );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnViewComponent::initialise( const Json::Value& Object )
{
	initialise();

	X_ = (BcF32)Object[ "x" ].asDouble();
	Y_ = (BcF32)Object[ "y" ].asDouble();
	Width_ = (BcF32)Object[ "width" ].asDouble();
	Height_ = (BcF32)Object[ "height" ].asDouble();
	Near_ = (BcF32)Object[ "near" ].asDouble();
	Far_ = (BcF32)Object[ "far" ].asDouble();
	HorizontalFOV_ = (BcF32)Object[ "hfov" ].asDouble();
	VerticalFOV_ = (BcF32)Object[ "vfov" ].asDouble();

	const Json::Value& RenderMaskValue = Object[ "rendermask" ];
	if( RenderMaskValue.type() != Json::nullValue )
	{
		setRenderMask( RenderMaskValue.asUInt() );
	}

	const Json::Value& RenderTargetValue = Object[ "rendertarget" ];
	if( RenderTargetValue.type() != Json::nullValue )
	{
		RenderTarget_ = getPackage()->getPackageCrossRef( RenderTargetValue.asUInt() );
	}
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnViewComponent::setMaterialParameters( ScnMaterialComponent* MaterialComponent ) const
{
	MaterialComponent->setClipTransform( Viewport_.view() * Viewport_.projection() );
	MaterialComponent->setViewTransform( Viewport_.view() );
	MaterialComponent->setEyePosition( InverseViewMatrix_.translation() );	
}

//////////////////////////////////////////////////////////////////////////
// getWorldPosition
void ScnViewComponent::getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far ) const
{
	// NOTE: Uses last viewport bound.
	Viewport_.unProject( ScreenPosition, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// bind
const RsViewport& ScnViewComponent::getViewport() const
{
	return Viewport_;
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnViewComponent::bind( RsFrame* pFrame, RsRenderSort Sort )
{
	RsContext* pContext = pFrame->getContext();

	// Calculate the viewport.
	BcF32 Width = static_cast< BcF32 >( pContext->getWidth() );
	BcF32 Height = static_cast< BcF32 >( pContext->getHeight() );

	// If we're using a render target, we want to use it for dimensions.
	if( RenderTarget_.isValid() )
	{
		Width = static_cast< BcF32 >( RenderTarget_->getWidth() );
		Height = static_cast< BcF32 >( RenderTarget_->getHeight() );
	}

	const BcF32 ViewWidth = Width_ * Width;
	const BcF32 ViewHeight = Height_ * Height;
	const BcF32 Aspect = ViewWidth / ViewHeight;

	// Setup the viewport.
	Viewport_.viewport( static_cast< BcU32 >( X_ * Width ),
	                    static_cast< BcU32 >( Y_ * Height ),
	                    static_cast< BcU32 >( ViewWidth ),
	                    static_cast< BcU32 >( ViewHeight ),
	                    Near_,
	                    Far_ );

	// Setup the view matrix.
	InverseViewMatrix_ = getParentEntity()->getMatrix();
	BcMat4d ViewMatrix( InverseViewMatrix_ );
	ViewMatrix.inverse();	
	Viewport_.view( ViewMatrix );
	
	// Setup the perspective projection.
	BcMat4d ProjectionMatrix;
	if( HorizontalFOV_ > 0.0f )
	{
		ProjectionMatrix.perspProjectionHorizontal( HorizontalFOV_, Aspect, Near_, Far_ );
	}
	else
	{
		ProjectionMatrix.perspProjectionVertical( VerticalFOV_, 1.0f / Aspect, Near_, Far_ );
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
// setRenderMask
void ScnViewComponent::setRenderMask( BcU32 RenderMask )
{
	RenderMask_ = RenderMask;
}

//////////////////////////////////////////////////////////////////////////
// getRenderMask
const BcU32 ScnViewComponent::getRenderMask() const
{
	return RenderMask_;
}
