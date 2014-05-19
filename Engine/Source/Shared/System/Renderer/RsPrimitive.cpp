/**************************************************************************
*
* File:		RsPrimitive.cpp
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsPrimitive.h"
#include "System/Renderer/RsVertexDeclaration.h"

////////////////////////////////////////////////////////////////////////////////
// RsPrimitiveDesc
RsPrimitiveDesc::RsPrimitiveDesc( class RsVertexDeclaration* VertexDeclaration )
{
	BcU32 MaxStream = 0;
	for( auto Element : VertexDeclaration->getDesc().Elements_ )
	{
		MaxStream = std::max( MaxStream, Element.StreamIdx_ );
	}

	VertexBuffers_.resize( MaxStream + 1, nullptr );
}

////////////////////////////////////////////////////////////////////////////////
// setVertexBuffer
RsPrimitiveDesc& RsPrimitiveDesc::setVertexBuffer( BcU32 StreamIdx, class RsVertexBuffer* VertexBuffer )
{
	VertexBuffers_[ StreamIdx ] = VertexBuffer;
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
// setIndexBuffer
RsPrimitiveDesc& RsPrimitiveDesc::setIndexBuffer( class RsIndexBuffer* IndexBuffer )
{
	IndexBuffer_ = IndexBuffer;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsPrimitive::RsPrimitive( class RsContext* pContext, const RsPrimitiveDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsPrimitive::~RsPrimitive()
{

}
