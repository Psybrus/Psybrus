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
// GaExampleComponentRef
typedef CsResourceRef< class GaWorldPressureComponent > GaWorldPressureComponentRef;


//////////////////////////////////////////////////////////////////////////
// GaWorldPressureSample
struct GaWorldPressureSample
{
	BcReal							Value_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldPressureVertex
struct GaWorldPressureVertex
{
	BcF32 X_, Y_, Z_;
	BcF32 U_, V_, W_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldPressureComponent
class GaWorldPressureComponent:
	public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, GaWorldPressureComponent );

public:
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	virtual void						update( BcReal Tick );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	void								addSample( const BcVec3d& Position, BcReal Value );
	void								setSample( const BcVec3d& Position, BcReal Value );

	eEvtReturn							onReset( EvtID ID, const GaWorldResetEvent& Event );

protected:
	GaWorldPressureSample&				sample( BcU32 Buffer, BcU32 X, BcU32 Y, BcU32 Z );

protected:
	void								updateSimulation();
	void								collideSimulation();
	void								updateTexture();
	void								updateGlowTextures();

private:
	SysFence							UpdateFence_;

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
	GaWorldBSPComponentRef				BSP_;
	
	struct TDynamicMaterial
	{
		ScnMaterialComponentRef			PreviewMaterial_;
		ScnMaterialComponentRef			WorldMaterial_;
		ScnTextureRef					WorldTexture1D_;
		ScnTextureRef					WorldTexture2D_;
		ScnTextureRef					WorldTexture3D_;
	};

	TDynamicMaterial					DynamicMaterials_[ 2 ];
	BcU32								CurrMaterial_;

	// Graphics data.
	BcBool								IsReady_;
	GaWorldPressureVertex*				pVertexArray_;
	RsVertexBuffer*						pVertexBuffer_;
	RsPrimitive*						pPrimitive_;
};

//////////////////////////////////////////////////////////////////////////
// sample
BcForceInline GaWorldPressureSample& GaWorldPressureComponent::sample( BcU32 Buffer, BcU32 X, BcU32 Y, BcU32 Z )
{
	const BcU32 Idx = X + ( Y + Z * Height_ ) * Width_;
#if 0
	BcAssertMsg( UpdateFence_.count() == 0, "Attempted access during an update!" );
	BcAssertMsg( Buffer < 2, "Buffer index out of bounds" );
	BcAssertMsg( Idx < BufferSize_, "Indices out of bounds" );
#endif
	return pBuffers_[ Buffer ][ Idx ];
}

#endif
