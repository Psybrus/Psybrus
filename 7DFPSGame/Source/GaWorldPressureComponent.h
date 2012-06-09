/**************************************************************************
*
* File:		GaWorldPressureComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World BSP
*		
*
*
* 
**************************************************************************/

#ifndef __GAWORLDPRESSURECOMPONENT_H__
#define __GAWORLDPRESSURECOMPONENT_H__

#include "Psybrus.h"

#include "GaWorldBSPComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaWorldPressureSample
struct GaWorldPressureSample
{
	BcReal							Value_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldPressureComponent
class GaWorldPressureComponent: public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaWorldPressureComponent );

public:
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	virtual void						update( BcReal Tick );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:
	GaWorldPressureSample&				sample( BcU32 Buffer, BcU32 X, BcU32 Y, BcU32 Z );

private:
	BcU32								Width_;
	BcU32								Height_;
	BcU32								Depth_;
	BcReal								AccumMultiplier_;
	BcReal								Damping_;

	BcReal								Scale_;
	BcVec2d								Offset_;

	GaWorldPressureSample*				pBuffers_[ 2 ];
	BcU32								BufferSize_;
	BcU32								CurrBuffer_;

	ScnCanvasComponentRef				Canvas_;
	ScnMaterialComponentRef				Material_;
	GaWorldBSPComponentRef				BSP_;
};

//////////////////////////////////////////////////////////////////////////
// sample
BcForceInline GaWorldPressureSample& GaWorldPressureComponent::sample( BcU32 Buffer, BcU32 X, BcU32 Y, BcU32 Z )
{
	const BcU32 Idx = X + ( Y + Z * Width_ ) * Height_;
#if 1
	BcAssertMsg( Buffer < 2, "Buffer index out of bounds" );
	BcAssertMsg( Idx < BufferSize_, "Indices out of bounds" );
#endif
	return pBuffers_[ Buffer ][ Idx ];
}

#endif
