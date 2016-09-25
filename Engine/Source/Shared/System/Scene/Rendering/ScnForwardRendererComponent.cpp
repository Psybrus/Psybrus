#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Scene/ScnCore.h"

#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnForwardRendererComponent.h"
#include "System/Scene/Rendering/ScnEnvironmentProbeComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsUtils.h"

#include "System/SysKernel.h"

#include "Editor/Editor.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// ScnForwardRendererComponent
REFLECTION_DEFINE_DERIVED( ScnForwardRendererComponent );

void ScnForwardRendererComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Enabled_", &ScnForwardRendererComponent::Enabled_, bcRFF_IMPORTER ),
		new ReField( "Width_", &ScnForwardRendererComponent::Width_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnForwardRendererComponent::Height_, bcRFF_IMPORTER ),
		new ReField( "ResolveX_", &ScnForwardRendererComponent::ResolveX_, bcRFF_IMPORTER ),
		new ReField( "ResolveY_", &ScnForwardRendererComponent::ResolveY_, bcRFF_IMPORTER ),
		new ReField( "ResolveW_", &ScnForwardRendererComponent::ResolveW_, bcRFF_IMPORTER ),
		new ReField( "ResolveH_", &ScnForwardRendererComponent::ResolveH_, bcRFF_IMPORTER ),
		new ReField( "Near_", &ScnForwardRendererComponent::Near_, bcRFF_IMPORTER ),
		new ReField( "Far_", &ScnForwardRendererComponent::Far_, bcRFF_IMPORTER ),
		new ReField( "HorizontalFOV_", &ScnForwardRendererComponent::HorizontalFOV_, bcRFF_IMPORTER ),
		new ReField( "VerticalFOV_", &ScnForwardRendererComponent::VerticalFOV_, bcRFF_IMPORTER ),
		new ReField( "ReflectionCubemap_", &ScnForwardRendererComponent::ReflectionCubemap_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "UseEnvironmentProbes_", &ScnForwardRendererComponent::UseEnvironmentProbes_, bcRFF_IMPORTER ),
	};	
	
	auto& Class = ReRegisterClass< ScnForwardRendererComponent, Super >( Fields );
	Class.addAttribute( new ScnComponentProcessor() );

		// Add editor.
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				ScnForwardRendererComponent* Value = (ScnForwardRendererComponent*)Object;
				if( Value != nullptr )
				{
					Editor::ObjectEditor( ThisFieldEditor, Value, Value->getClass(), Flags );
				}
			} ) );

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnForwardRendererComponent::ScnForwardRendererComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnForwardRendererComponent::~ScnForwardRendererComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnForwardRendererComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Subscribe for recreation after textures have been created.
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			recreateResources();
			return evtRET_PASS;
		} );

	// Create views.
	MainView_ = getParentEntity()->attach< ScnViewComponent >(
		"MainView", 0, nullptr, nullptr,
		0x1, ScnShaderPermutationFlags::RENDER_FORWARD, 
		RsRenderSortPassFlags::OPAQUE | RsRenderSortPassFlags::TRANSPARENT | RsRenderSortPassFlags::OVERLAY, 
		!!Enabled_ );

	MainView_->registerViewCallback( this );
	MainView_->setReflectionCubemap( ReflectionCubemap_ );
	MainView_->setClearParams( RsColour::BLACK, true, true, true );

	setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );

	// Recreate all resources.
	recreateResources();
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnForwardRendererComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	// Already detached.
	MainView_ = nullptr;
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnForwardRendererComponent::render( RsFrame* Frame, RsFrameBuffer* ResolveTarget, RsRenderSort Sort )
{
	RsFrameBuffer* OldResolveTarget = ResolveTarget_;
	ResolveTarget_ = ResolveTarget;
	ScnViewProcessor::pImpl()->renderView( MainView_, Frame, Sort );
	ResolveTarget_ = OldResolveTarget;
}

//////////////////////////////////////////////////////////////////////////
// setProjectionParams
void ScnForwardRendererComponent::setProjectionParams( BcF32 Near, BcF32 Far, BcF32 HorizonalFOV, BcF32 VerticalFOV )
{
	MainView_->setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );
}

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnForwardRendererComponent::recreateResources()
{
	// Reset view render data.
	ScnViewProcessor::pImpl()->resetViewRenderData( this );
}


//////////////////////////////////////////////////////////////////////////
// onViewDrawPreRender
void ScnForwardRendererComponent::onViewDrawPreRender( ScnRenderContext& RenderContext )
{
}

//////////////////////////////////////////////////////////////////////////
// onViewDrawPostRender
void ScnForwardRendererComponent::onViewDrawPostRender( ScnRenderContext& RenderContext )
{
#if !PSY_PRODUCTION
	if( Enabled_ && DsCore::pImpl() )
	{
		RsViewport Viewport;
		if( ResolveTarget_ )
		{
			auto FBDesc = ResolveTarget_->getDesc();
			auto Desc = FBDesc.RenderTargets_[ 0 ].Texture_->getDesc();
			Viewport = RsViewport( 
				BcU32( ResolveX_ * Desc.Width_ ),
				BcU32( ResolveY_ * Desc.Height_ ),
				BcU32( ResolveW_ * Desc.Width_ ),
				BcU32( ResolveH_ * Desc.Height_ ) );
		}
		else
		{
			auto Client = OsCore::pImpl()->getClient( 0 );
			Viewport = RsViewport( 
				BcU32( ResolveX_ * Client->getWidth() ),
				BcU32( ResolveY_ * Client->getHeight() ),
				BcU32( ResolveW_ * Client->getWidth() ),
				BcU32( ResolveH_ * Client->getHeight() ) );
		}

		DsCore::pImpl()->addViewOverlay( 
			MainView_->getViewTransform(), 
			MainView_->getProjectionTransform(), 
			Viewport );

		Debug::Render(
			RenderContext.pFrame_,
			RenderContext.View_->getFrameBuffer(),
			RenderContext.View_->getViewport(),
			RenderContext.View_->getViewUniformBuffer(),
			RenderContext.Sort_ );
	}
#endif
}
