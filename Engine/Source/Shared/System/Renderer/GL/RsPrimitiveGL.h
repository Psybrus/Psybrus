/**************************************************************************
*
* File:		RsPrimitiveGL.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPRIMITIVEGL_H__
#define __RSPRIMITIVEGL_H__

#include "System/Renderer/GL/RsVertexBufferGL.h"
#include "System/Renderer/GL/RsIndexBufferGL.h"

#include "System/Renderer/RsPrimitive.h"


////////////////////////////////////////////////////////////////////////////////
// RsPrimitiveGL
class RsPrimitiveGL:
	public RsPrimitive
{
public:
	/**
	 * Create primitive.<br/>
	 */
	RsPrimitiveGL( RsContext* pContext, const RsPrimitiveDesc& Desc );
	virtual ~RsPrimitiveGL();
	
private:
};

#endif
