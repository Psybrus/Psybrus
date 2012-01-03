/**************************************************************************
*
* File:		GaLayeredSpriteComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer for bunny ^_^
*		
*
*
* 
**************************************************************************/

#ifndef __GaLayeredSpriteComponent_H__
#define __GaLayeredSpriteComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class GaMainGameState;

////////////////////////////////////////////////////////////////////////////////
// GaLayeredSpriteComponentRef
typedef CsResourceRef< class GaLayeredSpriteComponent > GaLayeredSpriteComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaLayeredSpriteComponent
class GaLayeredSpriteComponent:
	public ScnComponent
{
public:
	enum
	{
		LAYER_FOOT_FRONT_0,
		LAYER_BODY,
		LAYER_EARS,
		LAYER_HEAD,
		LAYER_NOSE,
		LAYER_EYES,
		LAYER_FOOT_FRONT_1,
		LAYER_FOOT_REAR_0,
		LAYER_TAIL,
		
		//
		LAYER_MAX
	};

public:
	DECLARE_RESOURCE( ScnComponent, GaLayeredSpriteComponent );
	virtual void initialise( ScnMaterialRef Material, const BcVec3d& Scale );
	virtual void destroy();
	
	BcBool isReady();
	
	virtual void update( BcReal Tick );

	void render( GaMainGameState* pParent, ScnCanvasComponentRef Canvas, const BcVec3d& Position, const BcVec2d& Velocity );

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

private:
	ScnMaterialComponentRef MaterialComponent_;
	ScnMaterialComponentRef ShadowMaterialComponent_;

	struct TLayer
	{
		TLayer():
			TimeSpeed_( 1.0f ),
			TimeTicker_( 0.0f ),
			Multiplier_( 8.0f, 8.0f ),
			StaticMultiplier_( 8.0f, 8.0f )
		{
		}

		BcReal TimeSpeed_;
		BcReal TimeTicker_;
		BcVec2d Multiplier_;
		BcVec2d StaticMultiplier_;
	};

	BcVec3d Scale_;

	TLayer Layers_[ LAYER_MAX ];

	BcVec2d FaceDirection_;
	BcVec2d SmoothFaceDirection_;
};



#endif
