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
	ReflectionClass_( nullptr )
{

}
//////////////////////////////////////////////////////////////////////////
// RsUniformBufferDesc
RsUniformBufferDesc::RsUniformBufferDesc( const ReClass* ReflectionClass ):
	ReflectionClass_( ReflectionClass )
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

