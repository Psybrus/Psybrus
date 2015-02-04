/**************************************************************************
*
* File:		RsFrameBuffer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsFrameBuffer.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferDesc
RsFrameBufferDesc::RsFrameBufferDesc( BcU32 NoofTargets ):
	RenderTargets_( NoofTargets, nullptr ),
	DepthStencilTarget_( nullptr )
{
}


//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBuffer::RsFrameBuffer( RsContext* pContext, const RsFrameBufferDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsFrameBufferDesc& RsFrameBuffer::getDesc() const
{
	return Desc_;
}
