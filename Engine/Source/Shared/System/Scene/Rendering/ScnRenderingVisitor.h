/**************************************************************************
*
* File:		Rendering/ScnRenderingVisitor.h
* Author: 	Neil Richardson 
* Ver/Date:	27/12/2012
* Description:
*		Rendering visitor for the scenegraph.
*		
*
*
* 
**************************************************************************/

#ifndef __SCNRENDERINGVISITOR_H__
#define __SCNRENDERINGVISITOR_H__

#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderingVisitor
class ScnRenderingVisitor:
	public ScnVisitor
{
public:
	ScnRenderingVisitor( class ScnViewComponent* pViewComponent, class RsFrame* pFrame );
	virtual ~ScnRenderingVisitor();

	virtual void							visit( class ScnRenderableComponent* pComponent );

private:
	class ScnViewComponent*					pViewComponent_;
	class RsFrame*							pFrame_;
};

#endif
