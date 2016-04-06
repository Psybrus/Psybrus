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
	ScnRenderingVisitor();
	virtual ~ScnRenderingVisitor();

	virtual void visit( class ScnRenderableComponent* pComponent );

	void gatherVisible( class ScnRenderContext & RenderContext );
	void render( class ScnRenderContext & RenderContext );
	void clear();

private:
	class ScnRenderContext* RenderContext_ = nullptr;

	std::vector< ScnRenderableComponent* > VisibleComponents_;
};

#endif
