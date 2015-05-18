/**************************************************************************
*
* File:		RsBuffer.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSBUFFER_H__
#define __RSBUFFER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferDesc
struct RsBufferDesc
{
	RsBufferDesc();
	RsBufferDesc( RsBufferType Type, RsResourceCreationFlags Flags, size_t SizeBytes );
	
	RsBufferType Type_;
	RsResourceCreationFlags Flags_;
	size_t SizeBytes_;
};

//////////////////////////////////////////////////////////////////////////
// RsBuffer
class RsBuffer:
	public RsResource
{
public:
	RsBuffer( class RsContext* pContext, const RsBufferDesc& BufferDesc );
	virtual ~RsBuffer();
	
	const RsBufferDesc& getDesc() const;

private:
	RsBufferDesc BufferDesc_;
};

#endif
