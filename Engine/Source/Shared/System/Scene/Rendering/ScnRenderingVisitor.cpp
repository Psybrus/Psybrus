/**************************************************************************
*
* File:		ScnRenderingVisitor.cpp
* Author: 	Neil Richardson 
* Ver/Date:	27/12/2012
* Description:
*		Rendering visitor for the scenegraph.
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnRenderingVisitor.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Scene/ScnCore.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderingVisitor::ScnRenderingVisitor()
{
	// Reserve 32k visible components upfront to reduce chance of reallocation.
	VisibleComponents_.reserve( 1024 * 32 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnRenderingVisitor::~ScnRenderingVisitor()
{

}

//////////////////////////////////////////////////////////////////////////
// gatherVisible
void ScnRenderingVisitor::gatherVisible( class ScnRenderContext & RenderContext )
{
	BcAssert( RenderContext_ == nullptr );
	RenderContext_ = &RenderContext;
	ScnCore::pImpl()->visitView( this, RenderContext.pViewComponent_ );
	RenderContext_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnRenderingVisitor::render( class ScnRenderContext & RenderContext )
{
	BcAssert( RenderContext_ == nullptr );
	{
		PSY_PROFILER_SECTION( RootSort, "Sort visible components by type" );

		// Sort by type.
		std::sort( VisibleComponents_.begin(), VisibleComponents_.end(),
			[]( const ScnRenderableComponent* A, const ScnRenderableComponent* B )
			{
				return A->getClass() < B->getClass();
			} );
	}

	{
		PSY_PROFILER_SECTION( RootSort, "Render visible components" );

		// Iterate over components.
		for( auto Component : VisibleComponents_ )
		{
			auto* ViewRenderData = Component->getViewRenderData( RenderContext.pViewComponent_ );
			if( ViewRenderData )
			{
				RenderContext.ViewRenderData_ = ViewRenderData;
				RenderContext.Sort_.Pass_ = BcU64( ViewRenderData->getSortPassType() );
				Component->render( RenderContext );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// clear
void ScnRenderingVisitor::clear()
{
	VisibleComponents_.clear();
}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnRenderingVisitor::visit( class ScnRenderableComponent* pComponent )
{
	if( RenderContext_->pViewComponent_->getRenderMask() & pComponent->getRenderMask() )
	{
		PSY_LOGSCOPEDCATEGORY( *pComponent->getClass()->getName() );
		BcAssert( pComponent->isReady() );

		VisibleComponents_.push_back( pComponent );
	}
}
