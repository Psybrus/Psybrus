/**************************************************************************
*
* File:		RsUniformBuffer.h
* Author:	Neil Richardson 
* Ver/Date:	29/03/14	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSUNIFORMBUFFER_H__
#define __RSUNIFORMBUFFER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

/**
 * RsUniformBufferDesc
 */
struct RsUniformBufferDesc
{
	RsUniformBufferDesc();
	RsUniformBufferDesc( BcU32 BufferSize );

	BcU32 BufferSize_;
};

//////////////////////////////////////////////////////////////////////////
// RsUniformBuffer
class RsUniformBuffer:
	public RsResource
{
public:
	RsUniformBuffer( class RsContext* pContext );
	virtual ~RsUniformBuffer();

	/**
	 * Lock buffer.
	 */
	virtual void*						lock() = 0;

	/**
	 * Unlock buffer.
	 */
	virtual void						unlock() = 0;

};

#endif
