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
ScnRenderingVisitor::ScnRenderingVisitor( 
		class ScnViewComponent* pViewComponent, 
		class RsFrame* pFrame,
		RsRenderSort Sort ):
	pViewComponent_( pViewComponent ),
	pFrame_( pFrame ),
	Sort_( Sort )
{
	ScnCore::pImpl()->visitView( this, pViewComponent_ );
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
	if( pViewComponent_->getRenderMask() & pComponent->getRenderMask() )
	{
		BcAssert( pComponent->isReady() );
		pComponent->render( pViewComponent_, pFrame_, Sort_ );
	}
}
