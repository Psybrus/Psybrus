/**************************************************************************
*
* File:		RsPrimitiveGLES.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPRIMITIVEGLES_H__
#define __RSPRIMITIVEGLES_H__

#include "RsGPUResourceGLES.h"
#include "RsVertexBufferGLES.h"
#include "RsIndexBufferGLES.h"

#include "RsPrimitive.h"

////////////////////////////////////////////////////////////////////////////////
// RsPrimitiveGLES
class RsPrimitiveGLES:
	public RsPrimitive,
	public RsGPUResourceGLES
{
public:
	/**
	 * Create primitive.
	 * @param pVertexBuffer Vertex buffer.
	 * @param pIndexBuffer Index buffer.
	 */
	RsPrimitiveGLES( RsVertexBufferGLES* pVertexBuffer, RsIndexBufferGLES* pIndexBuffer );
	virtual ~RsPrimitiveGLES();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
public:
	virtual void						render( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices );
	
private:
	RsVertexBufferGLES*					pVertexBuffer_;
	RsIndexBufferGLES*					pIndexBuffer_;	
};

#endif
