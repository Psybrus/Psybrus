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

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCore.h"
#include "System/Scene/ScnEntity.h"

#include "System/Scene/Rendering/ScnRenderingVisitor.h"

#include "System/Debug/DsImGui.h"

#include "System/SysKernel.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnViewComponent );

void ScnViewComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "X_", &ScnViewComponent::X_, bcRFF_IMPORTER ),
		new ReField( "Y_", &ScnViewComponent::Y_, bcRFF_IMPORTER ),
		new ReField( "Width_", &ScnViewComponent::Width_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnViewComponent::Height_, bcRFF_IMPORTER ),
		new ReField( "Near_", &ScnViewComponent::Near_, bcRFF_IMPORTER ),
		new ReField( "Far_", &ScnViewComponent::Far_, bcRFF_IMPORTER ),
		new ReField( "HorizontalFOV_", &ScnViewComponent::HorizontalFOV_, bcRFF_IMPORTER ),
		new ReField( "VerticalFOV_", &ScnViewComponent::VerticalFOV_, bcRFF_IMPORTER ),
		new ReField( "ClearColour_", &ScnViewComponent::ClearColour_, bcRFF_IMPORTER ),
		new ReField( "EnableClearColour_", &ScnViewComponent::EnableClearColour_, bcRFF_IMPORTER ),
		new ReField( "EnableClearDepth_", &ScnViewComponent::EnableClearDepth_, bcRFF_IMPORTER ),
		new ReField( "EnableClearStencil_", &ScnViewComponent::EnableClearStencil_, bcRFF_IMPORTER ),
		new ReField( "RenderMask_", &ScnViewComponent::RenderMask_, bcRFF_IMPORTER ),
		new ReField( "RenderPermutation_", &ScnViewComponent::RenderPermutation_, bcRFF_IMPORTER ),
		new ReField( "Passes_", &ScnViewComponent::Passes_, bcRFF_IMPORTER | bcRFF_FLAGS ),
		new ReField( "RenderTarget_", &ScnViewComponent::RenderTarget_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "DepthStencilTarget_", &ScnViewComponent::DepthStencilTarget_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "Viewport_", &ScnViewComponent::Viewport_ ),
		new ReField( "ViewUniformBlock_", &ScnViewComponent::ViewUniformBlock_ ),
		new ReField( "ViewUniformBuffer_", &ScnViewComponent::ViewUniformBuffer_, bcRFF_TRANSIENT ),
		new ReField( "FrustumPlanes_", &ScnViewComponent::FrustumPlanes_ ),
	};
	
	using namespace std::placeholders;
	ReRegisterClass< ScnViewComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(		{
			ScnComponentProcessFuncEntry(
				"Render Views",
				ScnComponentPriority::VIEW_RENDER,
				std::bind( &ScnViewComponent::renderViews, _1 ) ),
		} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewComponent::ScnViewComponent():
	X_( 0 ),
	Y_( 0 ),
	Width_( 1.0f ),
	Height_( 1.0f ),
	Near_( 0.1f ),
	Far_( 1000.0f ),
	HorizontalFOV_( 0.0f ),
	VerticalFOV_( 0.0f ),
	ClearColour_( RsColour( 0.0f, 0.0f, 0.0f, 0.0f ) ),
	EnableClearColour_( true ),
	EnableClearDepth_( true ),
	EnableClearStencil_( true ),
	RenderMask_( 0 ),
	RenderPermutation_( ScnShaderPermutationFlags::RENDER_FORWARD ),
	Passes_( RsRenderSortPassFlags::OPAQUE | RsRenderSortPassFlags::TRANSPARENT ),
	RenderTarget_(),
	DepthStencilTarget_( nullptr )
{
	ViewUniformBuffer_ = nullptr;

	setRenderMask( 1 );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnViewComponent::~ScnViewComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnViewComponent::onAttach( ScnEntityWeakRef Parent )
{
	ViewUniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( ViewUniformBlock_ ) ),
		getFullName().c_str() );

	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			recreateFrameBuffer();
			return evtRET_PASS;
		} );

	memset( &ViewUniformBlock_, 0, sizeof( ViewUniformBlock_ ) );

	recreateFrameBuffer();
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnViewComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );
	ViewUniformBuffer_.reset();

	FrameBuffer_.reset();
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getViewUniformBuffer
class RsBuffer* ScnViewComponent::getViewUniformBuffer()
{
	return ViewUniformBuffer_.get();
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnViewComponent::setMaterialParameters( ScnMaterialComponent* MaterialComponent ) const
{
	MaterialComponent->setViewUniformBlock( ViewUniformBuffer_.get() );
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
// hasRenderTarget
BcBool ScnViewComponent::hasRenderTarget() const
{
	return RenderTarget_.size() > 0;
}

//////////////////////////////////////////////////////////////////////////
// getFrameBuffer
RsFrameBuffer* ScnViewComponent::getFrameBuffer() const
{
	return FrameBuffer_.get();
}

//////////////////////////////////////////////////////////////////////////
// getViewport
const RsViewport& ScnViewComponent::getViewport() const
{
	return Viewport_;
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnViewComponent::bind( RsFrame* pFrame, RsRenderSort Sort )
{
	// Calculate the viewport.
	BcF32 Width = static_cast< BcF32 >( pFrame->getBackBufferWidth() );
	BcF32 Height = static_cast< BcF32 >( pFrame->getBackBufferHeight() );

	// If we're using a render target, we want to use it for dimensions.
	if( RenderTarget_.size() > 0 )
	{
		Width = static_cast< BcF32 >( RenderTarget_[ 0 ]->getWidth() );
		Height = static_cast< BcF32 >( RenderTarget_[ 0 ]->getHeight() );
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
	ViewUniformBlock_.InverseClipTransform_ = ViewUniformBlock_.ClipTransform_;
	ViewUniformBlock_.InverseClipTransform_.inverse();

	// Time.
	BcF32 Time = SysKernel::pImpl()->getFrameTime();
	ViewUniformBlock_.ViewTime_ += MaVec4d( Time, Time * 0.5f, Time * 0.25f, Time * 0.125f );

	// View size.
	ViewUniformBlock_.ViewSize_ = MaVec4d( 
		static_cast< BcF32 >( Viewport_.width() ), 
		static_cast< BcF32 >( Viewport_.height() ),
		1.0f / static_cast< BcF32 >( Viewport_.width() ), 
		1.0f / static_cast< BcF32 >( Viewport_.height() ) );

	// Near + far planes.
	ViewUniformBlock_.NearFar_ = MaVec4d( Near_, Far_, Near_ + Far_, Near_ * Far_ );

	// Upload uniforms.
	RsCore::pImpl()->updateBuffer( 
		ViewUniformBuffer_.get(),
		0, sizeof( ViewUniformBlock_ ),
		RsResourceUpdateFlags::ASYNC,
		[
			ViewUniformBlock = ViewUniformBlock_
		]
		( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			BcMemCopy( Lock.Buffer_, &ViewUniformBlock, sizeof( ViewUniformBlock ) );
		} );

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

	// Setup render node to set the frame buffer, viewport, and clear colour.
	// TODO: Pass this in with the draw commands down the line.
	auto FrameBuffer = FrameBuffer_.get();
	auto Viewport = Viewport_;
	auto ClearColour = ClearColour_;
	auto EnableClearColour = EnableClearColour_;
	auto EnableClearDepth = EnableClearDepth_;
	auto EnableClearStencil = EnableClearStencil_;
	pFrame->queueRenderNode( Sort,
		[ FrameBuffer, Viewport, ClearColour, EnableClearColour, EnableClearDepth, EnableClearStencil ]( RsContext* Context )
		{
			PSY_PROFILER_SECTION( RenderRoot, "ScnViewComponentViewport::render" );
			Context->clear( FrameBuffer, ClearColour, EnableClearColour, EnableClearDepth, EnableClearStencil );
		} );
}

//////////////////////////////////////////////////////////////////////////
// recreate
void ScnViewComponent::recreateFrameBuffer()
{
	bool AnyValid = DepthStencilTarget_.isValid();
	for( auto RT : RenderTarget_ )
	{
		AnyValid |= RT.isValid();
	}

	if( AnyValid )
	{
		RsFrameBufferDesc FrameBufferDesc( RenderTarget_.size() );
		BcU32 RTIdx = 0;
		for( auto RT : RenderTarget_ )
		{
			FrameBufferDesc.setRenderTarget( RTIdx++, RT->getTexture() );
		}
		if( DepthStencilTarget_.isValid() )
		{
			FrameBufferDesc.setDepthStencilTarget( DepthStencilTarget_->getTexture() );
		}

		FrameBuffer_ = RsCore::pImpl()->createFrameBuffer( FrameBufferDesc, getFullName().c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
// renderViews
//static
void ScnViewComponent::renderViews( const ScnComponentList& Components )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnViewComponent::renderViews" ) );

	// TODO: Have things register with RsCore to be rendered maybe?

	// Get context.
	RsContext* pContext = RsCore::pImpl()->getContext( nullptr );

	// Check for native window handle, and a size greater than 0.
	// Don't want to render at all if none of those conditions are met.
	auto Client = pContext->getClient();
	if( Client->getWindowHandle() != 0 &&
		Client->getWidth() > 0 &&
		Client->getHeight() > 0 )
	{
		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		RsRenderSort Sort( 0 );
		
		// Check viewport count.
		if( Components.size() > RS_SORT_VIEWPORT_MAX )
		{
			PSY_LOG( "WARNING: More ScnViewComponents than there are availible slots. Reduce number of ScnViewComponents in scene or expect strange results." );
		}

		// Iterate over all view components.
		for( auto Component : Components )
		{
			BcAssert( Component->isTypeOf< ScnViewComponent >() );
			auto* ViewComponent = static_cast< ScnViewComponent* >( Component.get() );
	
			ScnRenderContext RenderContext( ViewComponent, pFrame, Sort );

			ViewComponent->bind( pFrame, Sort );

			ScnRenderingVisitor Visitor( RenderContext );

			// Increment viewport.
			Sort.Viewport_++;
		}

		// TODO: Move completely to DsCore.
		//       Probably depends on registration with RsCore.
		// Render ImGui.
		ImGui::Psybrus::Render( pContext, pFrame );

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
	}
}
