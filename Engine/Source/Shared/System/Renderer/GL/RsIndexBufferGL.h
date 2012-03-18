/**************************************************************************
 *
 * File:	RsIndexBufferGL.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		GL index buffer.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSINDEXBUFFERGL_H__
#define __RSINDEXBUFFERGL_H__

#include "System/Renderer/RsIndexBuffer.h"
#include "System/Renderer/GL/RsGL.h"

////////////////////////////////////////////////////////////////////////////////
// RsIndexBufferGL
class RsIndexBufferGL:
	public RsIndexBuffer
{
public:
	/**
	 * Create index buffer.
	 * @param NoofIndices Number of indices.
	 * @param pIndexData Pointer to index data, NULL to create own.
	 */
	RsIndexBufferGL( BcU32 NoofIndices, void* pIndexData );
	virtual ~RsIndexBufferGL();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

public:
	void								bind();
		
private:
	GLenum								Type_;
	GLenum								Usage_;

	BcU32								NoofIndices_;
	
};

#endif
