/**************************************************************************
*
* File:		RsVertex.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vertex stuff
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsVertex.h"

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

