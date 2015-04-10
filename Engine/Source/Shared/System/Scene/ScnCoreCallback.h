#pragma once

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnCoreCallback
class ScnCoreCallback
{
public:
	ScnCoreCallback(){}
	virtual ~ScnCoreCallback(){}

	/**
	 * Called after a component has been attached to the scene.
	 */
	virtual void onAttachComponent( class ScnComponent* Component ) = 0;

	/**
	 * Called before a component has been detached from the scene.
	 */
	virtual void onDetachComponent( class ScnComponent* Component ) = 0;
};
