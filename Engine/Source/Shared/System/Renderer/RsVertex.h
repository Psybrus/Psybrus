/**************************************************************************
*
* File:		RsVertex.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vertex stuff
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXDATA_H__
#define __RSVERTEXDATA_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsVertexElement
struct RsVertexElement
{
	RsVertexElement();

	RsVertexElement( BcU32 StreamIdx,
	                 BcU32 Offset,
	                 BcU32 Components,
	                 RsVertexDataType DataType,
	                 RsVertexUsage Usage,
	                 BcU32 UsageIdx );

	BcU32 getElementSize() const;


	BcU32				StreamIdx_;
	BcU32				Offset_;
	BcU32				Components_;
	RsVertexDataType	DataType_;
	RsVertexUsage		Usage_;
	BcU32				UsageIdx_;
};

#endif
