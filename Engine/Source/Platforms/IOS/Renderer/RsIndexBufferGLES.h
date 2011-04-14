/**************************************************************************
 *
 * File:	RsIndexBufferGLES.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		GLES index buffer.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSINDEXBUFFERGLES_H__
#define __RSINDEXBUFFERGLES_H__

#include "RsBufferGLES.h"
#include "RsIndexBuffer.h"

////////////////////////////////////////////////////////////////////////////////
// RsIndexBufferGLES
class RsIndexBufferGLES:
	public RsIndexBuffer,
	public RsBufferGLES
{
public:
	/**
	 * Create index buffer.
	 * @param NoofIndices Number of indices.
	 * @param pIndexData Pointer to index data, NULL to create own.
	 */
	RsIndexBufferGLES( BcU32 NoofIndices, void* pIndexData );
	virtual ~RsIndexBufferGLES();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

public:
	void								bind();
		
private:
	BcU32								NoofIndices_;
	
};

#endif
