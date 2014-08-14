/**************************************************************************
*
* File:		RsVertexDeclaration.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsVertexDeclaration.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// RsVertexDeclarationDesc
RsVertexDeclarationDesc::RsVertexDeclarationDesc( BcU32 NoofElements )
{
	if( NoofElements > 0 )
	{
		Elements_.reserve( NoofElements );
	}
}

//////////////////////////////////////////////////////////////////////////
// addElement
RsVertexDeclarationDesc& RsVertexDeclarationDesc::addElement( const RsVertexElement& Element )
{
	Elements_.push_back( Element );
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getHash
BcU32 RsVertexDeclarationDesc::getHash() const
{
	return BcHash::GenerateCRC32( 0, &Elements_[ 0 ], Elements_.size() * sizeof( Elements_[ 0 ] ) );
}

//////////////////////////////////////////////////////////////////////////
// getMinimumStride
BcU32 RsVertexDeclarationDesc::getMinimumStride() const
{
	BcU32 Stride = 0;

	for( const auto& Element : Elements_ )
	{
		Stride = BcMax( Stride, Element.Offset_ + Element.getElementSize() );
	}

	return Stride;
}


//////////////////////////////////////////////////////////////////////////
// Ctor
RsVertexDeclaration::RsVertexDeclaration( class RsContext* pContext, const RsVertexDeclarationDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
RsVertexDeclaration::~RsVertexDeclaration()
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsVertexDeclarationDesc& RsVertexDeclaration::getDesc() const
{
	return Desc_;
}
