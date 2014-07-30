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
// RsVertexBufferDesc
RsBufferDesc::RsBufferDesc( RsBufferType Type, RsBufferCreationFlags Flags, BcU32 SizeBytes ):
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

}
	
//////////////////////////////////////////////////////////////////////////
// getDesc
const RsBufferDesc& RsBuffer::getDesc() const
{
	return BufferDesc_;
}
