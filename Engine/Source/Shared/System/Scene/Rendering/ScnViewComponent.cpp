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

#include "System/Debug/DsImGui.h"

#include "System/SysKernel.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Processor

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewProcessor::ScnViewProcessor():
	ScnComponentProcessor( {
		ScnComponentProcessFuncEntry(
			"Render Views",
			ScnComponentPriority::VIEW_RENDER,
			std::bind( &ScnViewProcessor::renderViews, this, std::placeholders::_1 ) ) } )
{
	SpatialTree_.reset( new ScnViewVisibilityTree() );

	// Create root node for spatial tree.
	MaVec3d HalfBounds( MaVec3d( 32.0f, 32.0f, 32.0f ) * 1024.0f );
	SpatialTree_->createRoot( MaAABB( -HalfBounds, HalfBounds ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnViewProcessor::~ScnViewProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// registerRenderInterface
void ScnViewProcessor::registerRenderInterface( const ReClass* Class, ScnViewRenderInterface* Interface )
{
	BcAssert( RenderInterfaces_.find( Class ) == RenderInterfaces_.end() );
	RenderInterfaces_.insert( std::make_pair( Class, Interface ) );
}

//////////////////////////////////////////////////////////////////////////
// deregisterRenderInterface
void ScnViewProcessor::deregisterRenderInterface( const ReClass* Class, ScnViewRenderInterface* Interface )
{
	BcAssert( RenderInterfaces_.find( Class ) != RenderInterfaces_.end() );
	BcAssert( RenderInterfaces_.find( Class )->second == Interface );
	RenderInterfaces_.erase( RenderInterfaces_.find( Class ) );
}

//////////////////////////////////////////////////////////////////////////
// resetViewRenderData
void ScnViewProcessor::resetViewRenderData( class ScnComponent* Component )
{
	PendingViewDataReset_.insert( Component );
}

//////////////////////////////////////////////////////////////////////////
// renderViews
void ScnViewProcessor::renderViews( const ScnComponentList& Components )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnViewProcessor::renderViews" ) );

	if( PendingViewDataReset_.size() > 0 )
	{
		PSY_PROFILER_SECTION( RootSort, "Pending view data resets." );
		auto PendingViewDataReset = std::move( PendingViewDataReset_ );
		for( auto Component : PendingViewDataReset )
		{
			// Perform detach/attach to recreate the view data.
			onDetachComponent( Component );
			onAttachComponent( Component );	
		}
	}

	{
		PSY_PROFILER_SECTION( RootSort, "Update all components in visibility tree" );
		for( auto It : VisibilityLeaves_ )
		{
			auto* Leaf = It.second;
			BcAssert( Leaf->Node_ );
			BcAssert( Leaf->Component_ );
			MaAABB AABB;
			Leaf->RenderInterface_->getAABB( &AABB, &Leaf->Component_, 1 );
			if( Leaf->AABB_.min() != AABB.min() || Leaf->AABB_.max() != AABB.max() )
			{
				Leaf->AABB_ = AABB;
				Leaf->Node_->reinsertLeaf( Leaf );
			}
		}
	}

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

		// If we have no views, clear to black.
		if( Components.size() == 0 )
		{
			pFrame->queueRenderNode( Sort,
				[]( RsContext* Context )
				{
					Context->clear( nullptr, RsColour::BLACK, BcTrue, BcTrue, BcTrue );
				} );
		}

		// Iterate over all view components.
		BcAssert( Components.size() == ViewData_.size() );
		ScnViewComponent* LastView = nullptr;

		for( auto& ViewData : ViewData_ )
		{
			ScnRenderContext RenderContext( ViewData->View_, pFrame, Sort );
			bool DoGather = true;

			{
				PSY_PROFILER_SECTION( RootSort, "Check last view component for reuse." );

				if( LastView != nullptr && ViewData->View_ != LastView )
				{
					// Check current view
					if( LastView->compare( ViewData->View_ ) )
					{
						DoGather = false;
					}
				}
			}

			{
				PSY_PROFILER_SECTION( RootSort, "Setup view" );
				ViewData->View_->setup( pFrame, Sort );
			}

			if( DoGather )
			{
				PSY_PROFILER_SECTION( RootSort, "Gather all visible leaves." );

				GatheredVisibleLeaves_.clear();
				std::vector< ScnViewVisibilityLeaf* > BroadGather;
				SpatialTree_->gatherView( RenderContext.pViewComponent_, BroadGather );

				// Trim down based on render mask.
				for( auto Leaf : BroadGather )
				{
					if( RenderContext.pViewComponent_->getRenderMask() & Leaf->RenderMask_ )
					{
						GatheredVisibleLeaves_.push_back( Leaf );
					}
				}
			}

			{
				PSY_PROFILER_SECTION( RootSort, "Sort visible components by render interface" );

				// Sort by render interface.
				// TODO: Put into correct buckets by type.
				std::sort( GatheredVisibleLeaves_.begin(), GatheredVisibleLeaves_.end(),
					[ this ]( const ScnViewVisibilityLeaf* A, const ScnViewVisibilityLeaf* B )
					{
						return A->RenderInterface_ < B->RenderInterface_;
					} );
			}

			{
				PSY_PROFILER_SECTION( RootSort, "Build processing list." );

				ProcessingGroups_.clear();
				ViewComponentRenderDatas_.clear();
				if( GatheredVisibleLeaves_.size() > 0 )
				{
					ProcessingGroup Group;
					Group.RenderInterface_ = GatheredVisibleLeaves_[ 0 ]->RenderInterface_;
					for( BcU32 Idx = 0; Idx < GatheredVisibleLeaves_.size(); ++Idx )
					{
						ScnViewComponentRenderData ComponentRenderData;
						auto* Leaf = GatheredVisibleLeaves_[ Idx ];
						ComponentRenderData.Component_ = Leaf->Component_;

						// Find view render data.
						auto It = ViewData->ViewRenderData_.find( Leaf->Component_ );
						if( It != ViewData->ViewRenderData_.end() )
						{
							ComponentRenderData.ViewRenderData_ = It->second;
							if( Group.RenderInterface_ != Leaf->RenderInterface_ )
							{
								ProcessingGroups_.push_back( Group );
								Group.RenderInterface_ = Leaf->RenderInterface_;
								Group.Base_ = ViewComponentRenderDatas_.size();
								Group.Noof_ = 0;
							}
							Group.Noof_++;

							ViewComponentRenderDatas_.push_back( ComponentRenderData );
						}
					}

					if( Group.Noof_ > 0 )
					{
						ProcessingGroups_.push_back( Group );
					}
				}
			}

			{
				PSY_PROFILER_SECTION( RootSort, "Render visible components" );

				// Iterate over components.
				for( auto Group : ProcessingGroups_ )
				{
					Group.RenderInterface_->render( ViewComponentRenderDatas_.data() + Group.Base_, Group.Noof_, RenderContext );
				}
			}

			// Increment viewport.
			Sort.Viewport_++;

			// Set last view.
			LastView = ViewData->View_;
		}
		
		// TODO: Move completely to DsCore.
		//       Probably depends on registration with RsCore.
		// Render ImGui.
		ImGui::Psybrus::Render( pContext, pFrame );

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
	}
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnViewProcessor::initialise()
{
	ScnCore::pImpl()->addCallback( this );

	// Subscribe for resize event to reset bindings incase anything needs to be rebound.
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			auto VisibilityLeaves = VisibilityLeaves_;
			for( auto It : VisibilityLeaves )
			{
				ScnViewProcessor::pImpl()->resetViewRenderData( It.first );
			}
			return evtRET_PASS;
		} );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnViewProcessor::shutdown()
{
	OsCore::pImpl()->unsubscribeAll( this );
	ScnCore::pImpl()->removeCallback( this );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnViewProcessor::onAttach( ScnComponent* Component )
{
	BcAssert( Component->isTypeOf< ScnViewComponent >() );

	// Setup view data.
	std::unique_ptr< ViewData > ViewData( new ViewData );
	ViewData->View_ = static_cast< ScnViewComponent* >( Component );
#if 0
	ViewData->ClassData_.reserve( RenderInterfaces_.size() );
	for( auto It : RenderInterfaces_ )
	{
		ViewData::ClassData ClassData;
		ClassData.Class_ = It.first;
		ViewData->ClassData_.emplace_back( std::move( ClassData ) );
	}
#endif

	// Create view render data for all components.
	for( auto& It : VisibilityLeaves_ )
	{
		auto ViewRenderData = It.second->RenderInterface_->createViewRenderData( It.second->Component_, ViewData->View_ );
		if( ViewRenderData && ViewRenderData->getSortPassType() != RsRenderSortPassType::INVALID )
		{
			ViewData->ViewRenderData_[ It.second->Component_ ] = ViewRenderData;
		}
		else
		{
			It.second->RenderInterface_->destroyViewRenderData( It.second->Component_, ViewData->View_, ViewRenderData );
		}
	}

	ViewData_.emplace_back( std::move( ViewData ) );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnViewProcessor::onDetach( ScnComponent* Component )
{
	// Find and remove view data.
	auto It = std::find_if( ViewData_.begin(), ViewData_.end(), 
		[ Component ]( const std::unique_ptr< ViewData >& ViewData )
		{
			return ViewData->View_ == Component;
		} );
	BcAssert( It != ViewData_.end() );

	// Destroy view render data for all components.
	for( auto& VisibilityLeaf : VisibilityLeaves_ )
	{
		auto* ViewRenderData = (*It)->ViewRenderData_[ VisibilityLeaf.second->Component_ ];
		VisibilityLeaf.second->RenderInterface_->destroyViewRenderData( VisibilityLeaf.second->Component_, (*It)->View_, ViewRenderData );
	}

	ViewData_.erase( It );
}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void ScnViewProcessor::onAttachComponent( ScnComponent* Component )
{
	if( auto RenderInterface = getRenderInterface( Component->getClass() ) )
	{
		auto It = PendingViewDataReset_.find( Component );
		if( It != PendingViewDataReset_.end() )
		{
			PendingViewDataReset_.erase( It );
		}

		BcAssert( VisibilityLeaves_.find( Component ) == VisibilityLeaves_.end() );
		auto Leaf = new ScnViewVisibilityLeaf();
		Leaf->Node_ = nullptr;
		Leaf->Component_ = Component;
		Leaf->RenderInterface_ = RenderInterface;
		RenderInterface->getAABB( &Leaf->AABB_, &Component, 1 );
		RenderInterface->getRenderMask( &Leaf->RenderMask_, &Component, 1 );
		VisibilityLeaves_[ Component ] = Leaf;
		SpatialTree_->addLeaf( Leaf );
		BcAssert( Leaf->Node_ );

		// Create view render data for each view.
		for( auto& ViewData : ViewData_ )
		{
			auto ViewRenderData = RenderInterface->createViewRenderData( Component, ViewData->View_ );
			if( ViewRenderData && ViewRenderData->getSortPassType() != RsRenderSortPassType::INVALID )
			{
				ViewData->ViewRenderData_[ Component ] = ViewRenderData;
			}
			else
			{
				RenderInterface->destroyViewRenderData( Component, ViewData->View_, ViewRenderData );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnViewProcessor::onDetachComponent( ScnComponent* Component )
{
	if( auto RenderInterface = getRenderInterface( Component->getClass() ) )
	{
		auto It = PendingViewDataReset_.find( Component );
		if( It != PendingViewDataReset_.end() )
		{
			PendingViewDataReset_.erase( It );
		}

		auto LeafIt = VisibilityLeaves_.find( Component );
		BcAssert( LeafIt != VisibilityLeaves_.end() );
		SpatialTree_->removeLeaf( LeafIt->second );
		VisibilityLeaves_.erase( LeafIt );

		// Setup view render data for each view.
		for( auto& ViewData : ViewData_ )
		{
			auto ViewRenderDataIt = ViewData->ViewRenderData_.find( Component );
			if( ViewRenderDataIt != ViewData->ViewRenderData_.end() )
			{
				RenderInterface->destroyViewRenderData( Component, ViewData->View_, ViewRenderDataIt->second );
				ViewData->ViewRenderData_.erase( ViewRenderDataIt );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderInterface
ScnViewRenderInterface* ScnViewProcessor::getRenderInterface( const ReClass* Class )
{
	while( Class != nullptr )
	{
		decltype(RenderInterfaces_)::iterator It;
		if( ( It = RenderInterfaces_.find( Class ) ) != RenderInterfaces_.end() )
		{
			return It->second;
		}
		Class = Class->getSuper();
	}
	return nullptr;
}

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
		.addAttribute( new ScnViewProcessor() );
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
			RsResourceBindFlags::UNIFORM_BUFFER,
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
// compareFrustum
bool ScnViewComponent::compare( const ScnViewComponent* Other ) const
{
	if( getRenderMask() != Other->getRenderMask() ||
		ViewUniformBlock_.ClipTransform_.row0() != Other->ViewUniformBlock_.ClipTransform_.row0() ||
		ViewUniformBlock_.ClipTransform_.row1() != Other->ViewUniformBlock_.ClipTransform_.row1() ||
		ViewUniformBlock_.ClipTransform_.row2() != Other->ViewUniformBlock_.ClipTransform_.row2() ||
		ViewUniformBlock_.ClipTransform_.row3() != Other->ViewUniformBlock_.ClipTransform_.row3() )
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// getSortPassType
RsRenderSortPassType ScnViewComponent::getSortPassType( RsRenderSortPassFlags SortPassFlags, ScnShaderPermutationFlags PermutationFlags ) const
{
	RsRenderSortPassType RetVal = RsRenderSortPassType::INVALID;
	if( BcContainsAnyFlags( RenderPermutation_, PermutationFlags ) )
	{
		const auto CombinedFlags = Passes_ & SortPassFlags;
		const auto LeadingZeros = BcCountLeadingZeros( static_cast< BcU32 >( CombinedFlags ) );
		if( LeadingZeros < 32 )
		{
			RetVal = static_cast< RsRenderSortPassType >( 31 - LeadingZeros );
		}
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setup
void ScnViewComponent::setup( RsFrame* pFrame, RsRenderSort Sort )
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
		RsFrameBufferDesc FrameBufferDesc( static_cast< BcU32 >( RenderTarget_.size() ) );
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
