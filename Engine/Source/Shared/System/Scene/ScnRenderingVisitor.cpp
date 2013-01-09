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

#include "System/Scene/ScnRenderingVisitor.h"

#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderingVisitor::ScnRenderingVisitor( class ScnViewComponent* pViewComponent, class RsFrame* pFrame ):
	pViewComponent_( pViewComponent ),
	pFrame_( pFrame )
{
	ScnCore::pImpl()->visitView( this, pViewComponent_->getViewport() );
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
		pComponent->render( pViewComponent_, pFrame_, RsRenderSort( 0 ) );
	}
}
