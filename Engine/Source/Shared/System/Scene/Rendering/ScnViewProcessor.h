#pragma once

#include "System/Scene/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewProcessor
class ScnViewProcessor : public ScnComponentProcessor
{
public:
	ScnViewProcessor();

	/**
	 * Will render everything visible to all views.
	 */
	void renderViews( const ScnComponentList& Components );	

private:

};