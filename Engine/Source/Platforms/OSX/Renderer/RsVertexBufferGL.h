/**************************************************************************
*
* File:		RsVertexBufferGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL vertex buffer.
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXBUFFERGL_H__
#define __RSVERTEXBUFFERGL_H__

#include "RsVertexBuffer.h"
#include "RsGL.h"

#include "BcAtomicMutex.h"

////////////////////////////////////////////////////////////////////////////////
// RsVertexBufferGL
class RsVertexBufferGL:
	public RsVertexBuffer
{
public:
	/**
	 * Create vertex buffer.
	 * @param Descriptor Vertex descriptor flags.
	 * @param NoofVertices Number of vertices.
	 * @param pVertexData Pointer to vertex data, NULL to create own.
	 */
	RsVertexBufferGL( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData = NULL );
	virtual ~RsVertexBufferGL();

	virtual void*						lock();
	virtual void						unlock();
	
	virtual BcU32						getNoofVertices();
	virtual BcU32						getVertexFormat();

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
public:
	void								bind();
	
private:
	GLenum								Type_;
	GLenum								Usage_;

	BcU32								Descriptor_;
	BcU32								Stride_;
	BcU32								NoofVertices_;

	BcAtomicMutex						Lock_;
};

#endif
