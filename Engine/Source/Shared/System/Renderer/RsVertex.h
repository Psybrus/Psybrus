/**************************************************************************
*
* File:		RsVertex.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vertex stuff
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXDATA_H__
#define __RSVERTEXDATA_H__

#include "System/Renderer/RsTypes.h"

struct RsVertexElement
{
	BcU32				Components_;
	BcU32				Stride_;
	eRsVertexDataType	Type_;
	BcU32				StreamIdx_;
};


#endif
