/**************************************************************************
*
* File:		RsBuffer.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsBuffer.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferDesc
RsBufferDesc::RsBufferDesc():
	Type_( RsBufferType::UNKNOWN ),
	SizeBytes_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsBufferDesc
RsBufferDesc::RsBufferDesc( RsBufferType Type, RsResourceCreationFlags Flags, size_t SizeBytes ):
	Type_( Type ),
	Flags_( Flags ),
	SizeBytes_( SizeBytes )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBuffer::RsBuffer( class RsContext* pContext, const RsBufferDesc& BufferDesc ):
	RsResource( pContext ),
	BufferDesc_( BufferDesc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
RsBuffer::~RsBuffer()
{
#if PSY_DEBUG
	BufferDesc_.Type_ = RsBufferType::UNKNOWN;
	BufferDesc_.SizeBytes_ = BcErrorCode;
#endif
}
	
//////////////////////////////////////////////////////////////////////////
// getDesc
const RsBufferDesc& RsBuffer::getDesc() const
{
	return BufferDesc_;
}
