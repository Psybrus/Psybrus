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

#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderingVisitor::ScnRenderingVisitor( ScnRenderContext & RenderContext ):
	RenderContext_( RenderContext )
{
	ScnCore::pImpl()->visitView( this, RenderContext_.pViewComponent_ );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnRenderingVisitor::~ScnRenderingVisitor()
{

}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnRenderingVisitor::visit( class ScnRenderableComponent* pComponent )
{
	if( RenderContext_.pViewComponent_->getRenderMask() & pComponent->getRenderMask() )
	{
		PSY_LOGSCOPEDCATEGORY( *pComponent->getClass()->getName() );
		BcAssert( pComponent->isReady() );

		RenderContext_.ViewRenderData_ = pComponent->getViewRenderData( RenderContext_.pViewComponent_ );
		if( RenderContext_.ViewRenderData_ )
		{
			pComponent->render( RenderContext_ );
		}
	}
}
