/**************************************************************************
*
* File:		RsUniformBuffer.cpp
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsUniformBuffer.h"

//////////////////////////////////////////////////////////////////////////
// RsUniformBufferDesc
RsUniformBufferDesc::RsUniformBufferDesc():
	BufferSize_( 0 )
{

}
//////////////////////////////////////////////////////////////////////////
// RsUniformBufferDesc
RsUniformBufferDesc::RsUniformBufferDesc( BcU32 BufferSize ):
	BufferSize_( BufferSize )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsUniformBuffer::RsUniformBuffer( class RsContext* pContext ):
	RsResource( pContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsUniformBuffer::~RsUniformBuffer()
{
	
}

