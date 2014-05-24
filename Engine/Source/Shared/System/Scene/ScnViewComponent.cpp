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

#include "Base/BcMath.h"

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
		THeader Header = { pImage->width(), pImage->height(), 1, RsTextureFormat::RGBA8 };
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

void ScnViewComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "X_",					&ScnViewComponent::X_ ),
		ReField( "Y_",					&ScnViewComponent::Y_ ),
		ReField( "Width_",				&ScnViewComponent::Width_ ),
		ReField( "Height_",				&ScnViewComponent::Height_ ),
		ReField( "Near_",				&ScnViewComponent::Near_ ),
		ReField( "Far_",				&ScnViewComponent::Far_ ),
		ReField( "HorizontalFOV_",		&ScnViewComponent::HorizontalFOV_ ),
		ReField( "VerticalFOV_",		&ScnViewComponent::VerticalFOV_ ),
		ReField( "RenderMask_",			&ScnViewComponent::RenderMask_ ),
		ReField( "Viewport_",			&ScnViewComponent::Viewport_ ),
		ReField( "ViewUniformBlock_",	&ScnViewComponent::ViewUniformBlock_ ),
		ReField( "ViewUniformBuffer_",	&ScnViewComponent::ViewUniformBuffer_ ),
		ReField( "FrustumPlanes_",		&ScnViewComponent::FrustumPlanes_ ),
		ReField( "RenderTarget_",		&ScnViewComponent::RenderTarget_ ),
	};
		
	ReRegisterClass< ScnViewComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 2000 ) );
}

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
// create
//virtual
void ScnViewComponent::create()
{
	ScnComponent::create();
	ViewUniformBuffer_ = RsCore::pImpl()->createUniformBuffer( sizeof( ViewUniformBlock_ ), &ViewUniformBlock_ );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnViewComponent::destroy()
{
	RsCore::pImpl()->destroyResource( ViewUniformBuffer_ );

	ScnComponent::destroy();
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnViewComponent::setMaterialParameters( ScnMaterialComponent* MaterialComponent ) const
{
	MaterialComponent->setViewUniformBlock( ViewUniformBuffer_ );
}

//////////////////////////////////////////////////////////////////////////
// getWorldPosition
void ScnViewComponent::getWorldPosition( const MaVec2d& ScreenPosition, MaVec3d& Near, MaVec3d& Far ) const
{
	// TODO: Take normalised screen coordinates.
	MaVec2d Screen = ScreenPosition - MaVec2d( static_cast<BcF32>( Viewport_.x() ), static_cast<BcF32>( Viewport_.y() ) );
	const MaVec2d RealScreen( ( Screen.x() / Viewport_.width() ) * 2.0f - 1.0f, ( Screen.y() / Viewport_.height() ) * 2.0f - 1.0f );

	Near.set( RealScreen.x(), -RealScreen.y(), 0.0f );
	Far.set( RealScreen.x(), -RealScreen.y(), 1.0f );

	Near = Near * ViewUniformBlock_.InverseProjectionTransform_;
	Far = Far * ViewUniformBlock_.InverseProjectionTransform_;

	if( ViewUniformBlock_.ProjectionTransform_[3][3] == 0.0f )
	{
		Near *= Viewport_.zNear();
		Far *= Viewport_.zFar();
	}

	Near = Near * ViewUniformBlock_.InverseViewTransform_;
	Far = Far * ViewUniformBlock_.InverseViewTransform_;
}

//////////////////////////////////////////////////////////////////////////
// getScreenPosition
MaVec2d ScnViewComponent::getScreenPosition( const MaVec3d& WorldPosition ) const
{
	MaVec4d ScreenSpace = MaVec4d( WorldPosition, 1.0f ) * ViewUniformBlock_.ClipTransform_;
	MaVec2d ScreenPosition = MaVec2d( ScreenSpace.x() / ScreenSpace.w(), -ScreenSpace.y() / ScreenSpace.w() );

	BcF32 HalfW = BcF32( Viewport_.width() ) * 0.5f;
	BcF32 HalfH = BcF32( Viewport_.height() ) * 0.5f;
	return MaVec2d( ( ScreenPosition.x() * HalfW ), ( ScreenPosition.y() * HalfH ) );
}

//////////////////////////////////////////////////////////////////////////
// getDepth
BcU32 ScnViewComponent::getDepth( const MaVec3d& WorldPos ) const
{
	MaVec4d ScreenSpace = MaVec4d( WorldPos, 1.0f ) * ViewUniformBlock_.ClipTransform_;
	BcF32 Depth = 1.0f - BcClamp( ScreenSpace.z() / ScreenSpace.w(), 0.0f, 1.0f );

	return BcU32( Depth * BcF32( 0xffffff ) );
}

//////////////////////////////////////////////////////////////////////////
// getViewport
const RsViewport& ScnViewComponent::getViewport() const
{
	return Viewport_;
}


//////////////////////////////////////////////////////////////////////////
// intersect
BcBool ScnViewComponent::intersect( const MaAABB& AABB ) const
{
	MaVec3d Centre = AABB.centre();
	BcF32 Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

	BcF32 Distance;
	for( BcU32 i = 0; i < 6; ++i )
	{
		Distance = FrustumPlanes_[ i ].distance( Centre );
		if( Distance > Radius )
		{
			return BcFalse;
		}
	}

	return BcTrue;
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
	
	// Setup matrices in view uniform block.
	if( ViewUniformBuffer_->lock() )
	{
		// Create appropriate projection matrix.
		if( HorizontalFOV_ > 0.0f )
		{
			ViewUniformBlock_.ProjectionTransform_.perspProjectionHorizontal( HorizontalFOV_, Aspect, Near_, Far_ );
		}
		else
		{
			ViewUniformBlock_.ProjectionTransform_.perspProjectionVertical( VerticalFOV_, 1.0f / Aspect, Near_, Far_ );
		}

		ViewUniformBlock_.InverseProjectionTransform_ = ViewUniformBlock_.ProjectionTransform_;
		ViewUniformBlock_.InverseProjectionTransform_.inverse();

		// Setup view matrix.
		ViewUniformBlock_.InverseViewTransform_ = getParentEntity()->getWorldMatrix();
		ViewUniformBlock_.ViewTransform_ = ViewUniformBlock_.InverseViewTransform_;
		ViewUniformBlock_.ViewTransform_.inverse();

		// Clip transform.
		ViewUniformBlock_.ClipTransform_ = ViewUniformBlock_.ViewTransform_ * ViewUniformBlock_.ProjectionTransform_;

		ViewUniformBuffer_->unlock();
	}

	// Build frustum planes.
	// TODO: revisit this later as we don't need to do it I don't think.
	FrustumPlanes_[ 0 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] + ViewUniformBlock_.ClipTransform_[0][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] + ViewUniformBlock_.ClipTransform_[1][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] + ViewUniformBlock_.ClipTransform_[2][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] + ViewUniformBlock_.ClipTransform_[3][0]) );

	FrustumPlanes_[ 1 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] - ViewUniformBlock_.ClipTransform_[0][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] - ViewUniformBlock_.ClipTransform_[1][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] - ViewUniformBlock_.ClipTransform_[2][0] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] - ViewUniformBlock_.ClipTransform_[3][0] ) );

	FrustumPlanes_[ 2 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] + ViewUniformBlock_.ClipTransform_[0][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] + ViewUniformBlock_.ClipTransform_[1][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] + ViewUniformBlock_.ClipTransform_[2][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] + ViewUniformBlock_.ClipTransform_[3][1] ) );

	FrustumPlanes_[ 3 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] - ViewUniformBlock_.ClipTransform_[0][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] - ViewUniformBlock_.ClipTransform_[1][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] - ViewUniformBlock_.ClipTransform_[2][1] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] - ViewUniformBlock_.ClipTransform_[3][1] ) );

	FrustumPlanes_[ 4 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] - ViewUniformBlock_.ClipTransform_[0][2] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] - ViewUniformBlock_.ClipTransform_[1][2] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] - ViewUniformBlock_.ClipTransform_[2][2] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] - ViewUniformBlock_.ClipTransform_[3][2] ) );
	
	FrustumPlanes_[ 5 ] = MaPlane( ( ViewUniformBlock_.ClipTransform_[0][3] ),
	                               ( ViewUniformBlock_.ClipTransform_[1][3] ),
	                               ( ViewUniformBlock_.ClipTransform_[2][3] ),
	                               ( ViewUniformBlock_.ClipTransform_[3][3] ) );

	// Normalise frustum planes.
	for ( BcU32 i = 0; i < 6; ++i )
	{
		MaVec3d Normal = FrustumPlanes_[ i ].normal();
		BcF32 Scale = 1.0f / -Normal.magnitude();
		FrustumPlanes_[ i ] = MaPlane( FrustumPlanes_[ i ].normal().x() * Scale,
		                               FrustumPlanes_[ i ].normal().y() * Scale,
		                               FrustumPlanes_[ i ].normal().z() * Scale,
		                               FrustumPlanes_[ i ].d() * Scale );
	}

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
