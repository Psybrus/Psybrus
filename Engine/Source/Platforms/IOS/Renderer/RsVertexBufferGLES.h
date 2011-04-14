/**************************************************************************
*
* File:		RsVertexBufferGLES.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GLES vertex buffer.
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXBUFFERGLES_H__
#define __RSVERTEXBUFFERGLES_H__

#include "RsBufferGLES.h"
#include "RsVertexBuffer.h"

////////////////////////////////////////////////////////////////////////////////
// RsVertexBufferGLES
class RsVertexBufferGLES:
	public RsVertexBuffer,
	public RsBufferGLES
{
public:
	/**
	 * Create vertex buffer.
	 * @param Descriptor Vertex descriptor flags.
	 * @param NoofVertices Number of vertices.
	 * @param pVertexData Pointer to vertex data, NULL to create own.
	 */
	RsVertexBufferGLES( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData = NULL );
	virtual ~RsVertexBufferGLES();

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
public:
	void								bind();
	
private:
	BcU32								Descriptor_;
	BcU32								Stride_;
	BcU32								NoofVertices_;
};

#endif
