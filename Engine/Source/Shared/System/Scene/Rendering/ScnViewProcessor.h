#pragma once

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/Rendering/ScnRenderingVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewProcessor
class ScnViewProcessor : public ScnComponentProcessor
{
public:
	ScnViewProcessor();
	virtual ~ScnViewProcessor();

	/**
	 * Will render everything visible to all views.
	 */
	void renderViews( const ScnComponentList& Components );	

private:
	ScnRenderingVisitor RenderingVisitor_;


};