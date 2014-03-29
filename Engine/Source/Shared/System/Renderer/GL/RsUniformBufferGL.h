/**************************************************************************
 *
 * File:	RsUniformBufferGL.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		GL uniform buffer.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSUNIFORMBUFFERGL_H__
#define __RSUNIFORMBUFFERGL_H__

#include "System/Renderer/RsUniformBuffer.h"
#include "System/Renderer/GL/RsGL.h"

////////////////////////////////////////////////////////////////////////////////
// RsIndexBufferGL
class RsUniformBufferGL:
	public RsUniformBuffer
{
public:
	/**
	 * Create index buffer.
	 * @param BufferSize Buffer size.
	 * @param pBufferData Pointer to index data, NULL to create own.
	 */
	RsUniformBufferGL( RsContext* pContext, BcU32 BufferSize, void* pBufferData );
	virtual ~RsUniformBufferGL();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();

	virtual void*						lock();
	virtual void						unlock();
		
private:
	GLenum								Type_;
	GLenum								Usage_;
};

#endif
