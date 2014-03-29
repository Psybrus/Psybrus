/**************************************************************************
*
* File:		RsPrimitive.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPRIMITIVE_H__
#define __RSPRIMITIVE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsPrimitive
class RsPrimitive:
	public RsResource
{
public:
	RsPrimitive( class RsContext* pContext );
	virtual ~RsPrimitive();

	/**
	 * Render primitive.
	 * @param PrimitiveType Type of primitive to draw.
	 * @param Offset Offset into buffer to start primitive at.
	 * @param NoofIndices Number of indices to render.
	 */
	virtual void					render( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices ) = 0;

};

#endif
