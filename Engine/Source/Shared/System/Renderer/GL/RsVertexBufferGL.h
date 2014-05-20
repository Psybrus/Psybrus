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

#include "System/Renderer/RsVertexBuffer.h"
#include "System/Renderer/GL/RsGL.h"

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
	RsVertexBufferGL( RsContext* pContext, const RsVertexBufferDesc& Desc, void* pVertexData = NULL );
	virtual ~RsVertexBufferGL();

	virtual void*						lock();
	virtual void						unlock();
	
	virtual void						setNoofUpdateVertices( BcU32 NoofVertices );
	virtual BcU32						getNoofVertices() const;
	virtual BcU32						getVertexStride() const;

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
public:
	void								bind();
	
private:
	GLenum								Type_;
	GLenum								Usage_;

	RsVertexBufferDesc					Desc_;

	BcU32								NoofUpdateVertices_;
	BcBool								Created_;
};

#endif
