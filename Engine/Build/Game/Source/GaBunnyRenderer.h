/**************************************************************************
*
* File:		GaBunnyRenderer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer for bunny ^_^
*		
*
*
* 
**************************************************************************/

#ifndef __GABUNNYRENDERER_H__
#define __GABUNNYRENDERER_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaBunnyRenderer
class GaBunnyRenderer
{
public:
	enum
	{
		LAYER_FOOT_FRONT_0 = 0,
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
	GaBunnyRenderer();
	~GaBunnyRenderer();
	
	BcBool isReady();
	
	void update( BcReal Tick );

	void render( ScnCanvasRef Canvas, const BcVec2d& Position );

private:
	ScnMaterialInstanceRef MaterialInstance_;

	struct TLayer
	{
		TLayer():
			TimeSpeed_( 1.0f ),
			TimeTicker_( 0.0f ),
			Multiplier_( 8.0f, 8.0f )
		{
		}

		BcReal TimeSpeed_;
		BcReal TimeTicker_;
		BcVec2d Multiplier_;
	};

	TLayer Layers_[ LAYER_MAX ];
	

};



#endif
