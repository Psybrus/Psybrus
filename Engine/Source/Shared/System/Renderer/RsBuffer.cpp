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
RsBufferDesc::RsBufferDesc()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferDesc::RsBufferDesc( RsResourceBindFlags BindFlags, RsResourceCreationFlags Flags, size_t SizeBytes, size_t StructureStride ):
	BindFlags_( BindFlags ),
	Flags_( Flags ),
	SizeBytes_( SizeBytes ),
	StructureStride_( StructureStride )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBuffer::RsBuffer( class RsContext* pContext, const RsBufferDesc& BufferDesc ):
	RsResource( RsResourceType::BUFFER, pContext ),
	BufferDesc_( BufferDesc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
RsBuffer::~RsBuffer()
{
#if PSY_DEBUG
	BufferDesc_ = RsBufferDesc();
	BufferDesc_.SizeBytes_ = BcErrorCode;
#endif
}
	
//////////////////////////////////////////////////////////////////////////
// getDesc
const RsBufferDesc& RsBuffer::getDesc() const
{
	return BufferDesc_;
}
