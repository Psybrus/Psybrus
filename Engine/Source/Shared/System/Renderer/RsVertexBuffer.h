/**************************************************************************
*
* File:		RsVertexBuffer.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXBUFFER_H__
#define __RSVERTEXBUFFER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

/**
 * RsVertexBufferDesc
 */
struct RsVertexBufferDesc
{
	RsVertexBufferDesc();
	RsVertexBufferDesc( BcU32 Descriptor, BcU32 NoofVertices );
	RsVertexBufferDesc( BcU32 Descriptor, BcU32 NoofVertices, BcU32 Stride );

	BcU32 Descriptor_;
	BcU32 NoofVertices_;
	BcU32 Stride_;
};

//////////////////////////////////////////////////////////////////////////
// RsVertexBuffer
class RsVertexBuffer:
	public RsResource
{
public:
	RsVertexBuffer( class RsContext* pContext );
	virtual ~RsVertexBuffer();

	/**
	 * Lock buffer.
	 * NOTE: This will be better if we can specify range of vertices to change,
	 *       as we can then only change a small area of the buffer easily.
	 * @return Pointer to buffer.
	 */
	virtual void*						lock() = 0;
	
	/**
	 * Unlock buffer.
	 */
	virtual void						unlock() = 0;

	/**
	 * Set number of vertices to update.
	 */
	virtual void						setNoofUpdateVertices( BcU32 NoofVertices ) = 0;
	
	/**
	 *  Get number of vertices.
	 */
	virtual BcU32						getNoofVertices() = 0;
	
	/**
	 * Get vertex format.
	 */
	virtual BcU32						getVertexFormat() = 0;
};

#endif
