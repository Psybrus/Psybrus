/**************************************************************************
*
* File:		ScnLightingVisitor.h
* Author: 	Neil Richardson 
* Ver/Date:	13/01/13
* Description:
*		Lighting visitor for the scenegraph.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnLightingVisitor_H__
#define __ScnLightingVisitor_H__

#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnLightingVisitor
class ScnLightingVisitor:
	public ScnVisitor
{
public:
	enum
	{
		MAX_LIGHTS = 16
	};

public:
	ScnLightingVisitor( class ScnRenderableComponent* RenderableComponent );
	virtual ~ScnLightingVisitor();

	virtual void							visit( class ScnLightComponent* Component );

	void									setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;

private:
	std::vector< ScnLightComponent*	>		LightComponents_;
};

#endif
