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
// Ctor
RsVertexElement::RsVertexElement()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsVertexElement::RsVertexElement( BcU32 StreamIdx,
	                BcU32 Offset,
	                BcU32 Components,
	                RsVertexDataType DataType,
	                RsVertexUsage Usage,
	                BcU32 UsageIdx ):
	StreamIdx_( StreamIdx ),
	Offset_( Offset ),
	Components_( Components ),
	DataType_( DataType ),
	Usage_( Usage ),
	UsageIdx_( UsageIdx )
{
}

//////////////////////////////////////////////////////////////////////////
// getElementSize
BcU32 RsVertexElement::getElementSize() const
{
	BcU32 TypeSize = 0;

	switch( DataType_ )
	{
	case RsVertexDataType::FLOAT32:
		TypeSize = 4;
		break;
	case RsVertexDataType::FLOAT16:
		TypeSize = 2;
		break;
	case RsVertexDataType::FIXED:
		TypeSize = 2;
		break;
	case RsVertexDataType::BYTE:
	case RsVertexDataType::BYTE_NORM:
	case RsVertexDataType::UBYTE:
	case RsVertexDataType::UBYTE_NORM:
		TypeSize = 1;
		break;
	case RsVertexDataType::SHORT:
	case RsVertexDataType::SHORT_NORM:
	case RsVertexDataType::USHORT:
	case RsVertexDataType::USHORT_NORM:
		TypeSize = 2;
		break;
	case RsVertexDataType::INT:
	case RsVertexDataType::INT_NORM:
	case RsVertexDataType::UINT:
	case RsVertexDataType::UINT_NORM:
		TypeSize = 4;
		break;
	default:
		BcAssert( false );
	}

	return Components_ * TypeSize;
}

//////////////////////////////////////////////////////////////////////////
// RsVertexElement
bool RsVertexElement::operator ==( const RsVertexElement& Other ) const
{
	return StreamIdx_ == Other.StreamIdx_ &&
		Offset_ == Other.Offset_ &&
		Components_ == Other.Components_ &&
		DataType_ == Other.DataType_ &&
		Usage_ == Other.Usage_ &&
		UsageIdx_ == Other.UsageIdx_;
}

//////////////////////////////////////////////////////////////////////////
// RsVertexDeclarationDesc
bool RsVertexElement::operator !=( const RsVertexElement& Other ) const
{
	return !(*this == Other);
}

//////////////////////////////////////////////////////////////////////////
// RsVertexDeclarationDesc
RsVertexDeclarationDesc::RsVertexDeclarationDesc( size_t NoofElements )
{
	if( NoofElements > 0 )
	{
		Elements_.reserve( NoofElements );
	}
}

//////////////////////////////////////////////////////////////////////////
// RsVertexDeclarationDesc
RsVertexDeclarationDesc::RsVertexDeclarationDesc( RsVertexElement* Elements, size_t NoofElements ):
	RsVertexDeclarationDesc( NoofElements )
{
	for( size_t Idx = 0; Idx < NoofElements; ++Idx )
	{
		addElement( Elements[ Idx ] );
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
// operator ==
bool RsVertexDeclarationDesc::operator ==( const RsVertexDeclarationDesc& Other ) const
{
	if( Elements_.size() == Other.Elements_.size() )
	{
		for( size_t Idx = 0; Idx < Elements_.size(); ++Idx )
		{
			if( Elements_[ Idx ] != Other.Elements_[ Idx ] )
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// operator !=
bool RsVertexDeclarationDesc::operator !=( const RsVertexDeclarationDesc& Other ) const
{
	return !( *this == Other );
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
	RsResource( RsResourceType::VERTEX_DECLARATION, pContext ),
	Desc_( Desc )
{
	InputLayoutHash_ = BcHash::GenerateCRC32( 0, &Desc_.Elements_[ 0 ], sizeof( RsVertexElement ) * Desc_.Elements_.size() );
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

//////////////////////////////////////////////////////////////////////////
// getInputLayoutHash
BcU32 RsVertexDeclaration::getInputLayoutHash() const
{
	return InputLayoutHash_;
}
