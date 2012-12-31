/**************************************************************************
*
* File:		RsVertex.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vertex Descriptors
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEX_H__
#define __RSVERTEX_H__

#include "Base/BcTypes.h"
#include "Base/BcVectors.h"

struct RsVertex2D
{
	BcVec2d		Position_;
	BcVec2d		UV_;
	BcF32		R_, G_, B_, A_;
};

struct RsVertex3D
{
	BcVec3d		Position_;
	BcVec3d		Normal_;
	BcVec2d		UV_;
	BcF32		R_, G_, B_, A_;
};

#endif
