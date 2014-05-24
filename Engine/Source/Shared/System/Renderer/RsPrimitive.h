/**************************************************************************
*
* File:		RsPrimitive.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPRIMITIVE_H__
#define __RSPRIMITIVE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsPrimitiveDesc
struct RsPrimitiveDesc
{
	RsPrimitiveDesc( class RsVertexDeclaration* VertexDeclaration );

	RsPrimitiveDesc& setVertexBuffer( BcU32 StreamIdx, class RsVertexBuffer* VertexBuffer );
	RsPrimitiveDesc& setIndexBuffer( class RsIndexBuffer* IndexBuffer );

	class RsVertexDeclaration* VertexDeclaration_;
	std::vector< class RsVertexBuffer* > VertexBuffers_;
	class RsIndexBuffer* IndexBuffer_;
};

////////////////////////////////////////////////////////////////////////////////
// RsPrimitive
class RsPrimitive:
	public RsResource
{
public:
	RsPrimitive( class RsContext* pContext, const RsPrimitiveDesc& Desc );
	virtual ~RsPrimitive();

	/**
	 * Get descriptor.
	 */
	const RsPrimitiveDesc&			getDesc() const;

protected:
	RsPrimitiveDesc Desc_;
};

#endif
