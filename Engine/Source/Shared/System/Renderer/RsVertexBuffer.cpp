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
	Descriptor_( 0 ),
	NoofVertices_( 0 ),
	Stride_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsVertexBufferDesc
RsVertexBufferDesc::RsVertexBufferDesc( BcU32 Descriptor, BcU32 NoofVertices ):
	Descriptor_( Descriptor ),
	NoofVertices_( NoofVertices ),
	Stride_( RsVertexDeclSize( Descriptor ) )
{

}

//////////////////////////////////////////////////////////////////////////
// RsVertexBufferDesc
RsVertexBufferDesc::RsVertexBufferDesc( BcU32 Descriptor, BcU32 NoofVertices, BcU32 Stride ):
	Descriptor_( Descriptor ),
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

