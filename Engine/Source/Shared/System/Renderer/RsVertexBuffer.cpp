/**************************************************************************
*
* File:		RsVertexBuffer.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsVertexBuffer.h"

//////////////////////////////////////////////////////////////////////////
// RsVertexBufferDesc
RsVertexBufferDesc::RsVertexBufferDesc():
	NoofVertices_( 0 ),
	Stride_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsVertexBufferDesc
RsVertexBufferDesc::RsVertexBufferDesc( BcU32 NoofVertices ):
	NoofVertices_( NoofVertices )
{

}

//////////////////////////////////////////////////////////////////////////
// RsVertexBufferDesc
RsVertexBufferDesc::RsVertexBufferDesc( BcU32 NoofVertices, BcU32 Stride ):
	NoofVertices_( NoofVertices ),
	Stride_( Stride )
{

}


//////////////////////////////////////////////////////////////////////////
// Ctor
RsVertexBuffer::RsVertexBuffer( class RsContext* pContext ):
	RsResource( pContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsVertexBuffer::~RsVertexBuffer()
{
	
}

