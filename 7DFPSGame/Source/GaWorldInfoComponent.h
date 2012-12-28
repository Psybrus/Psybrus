/**************************************************************************
*
* File:		GaWorldInfoComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		World info component.
*		
*
*
* 
**************************************************************************/

#ifndef __GAWORLDINFOCOMPONENT_H__
#define __GAWORLDINFOCOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaWorldInfoComponentRef
typedef CsResourceRef< class GaWorldInfoComponent > GaWorldInfoComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaWorldInfoComponent
class GaWorldInfoComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaWorldInfoComponent );

	void								initialise( const Json::Value& Object );

	const BcU32							getWidth() const;
	const BcU32							getHeight() const;
	const BcU32							getDepth() const;
	const BcF32						getScale() const;
	const BcF32						getDamping() const;
	const BcF32						getAccumulatorMultiplier() const;

private:
	BcU32 Width_;
	BcU32 Height_;
	BcU32 Depth_;
	BcF32 Scale_;
	BcF32 Damping_;
	BcF32 AccumulatorMultiplier_;
};


#endif

