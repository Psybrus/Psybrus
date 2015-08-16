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
#include "System/Renderer/RsRenderNode.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderingVisitor
class ScnRenderingVisitor:
	public ScnVisitor
{
public:
	ScnRenderingVisitor( class ScnRenderContext & RenderContext );
	virtual ~ScnRenderingVisitor();

	virtual void visit( class ScnRenderableComponent* pComponent );

private:
	class ScnRenderContext & RenderContext_;
};

#endif
