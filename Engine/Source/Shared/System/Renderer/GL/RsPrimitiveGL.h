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
	 * TODO: Support multiple vertex buffers and binding by mask.
	 * @param pVertexBuffer Vertex buffer.
	 * @param pIndexBuffer Index buffer.
	 */
	RsPrimitiveGL( RsVertexBufferGL* pVertexBuffer, RsIndexBufferGL* pIndexBuffer );
	virtual ~RsPrimitiveGL();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
public:
	virtual void						render( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices );
	
private:
	RsVertexBufferGL*					pVertexBuffer_;
	RsIndexBufferGL*					pIndexBuffer_;	

	BcU32								VAO_;

};

#endif
