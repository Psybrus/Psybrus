/**************************************************************************
*
* File:		RsIndexBuffer.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsIndexBuffer.h"

//////////////////////////////////////////////////////////////////////////
// RsIndexBufferDesc
RsIndexBufferDesc::RsIndexBufferDesc():
	NoofIndices_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsIndexBufferDesc
RsIndexBufferDesc::RsIndexBufferDesc( BcU32 NoofIndices ):
	NoofIndices_( NoofIndices )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsIndexBuffer::RsIndexBuffer( class RsContext* pContext ):
	RsResource( pContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsIndexBuffer::~RsIndexBuffer()
{
	
}

