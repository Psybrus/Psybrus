/**************************************************************************
*
* File:		RsIndexBuffer.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSINDEXBUFFER_H__
#define __RSINDEXBUFFER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

/**
 * RsIndexBufferDesc
 */
struct RsIndexBufferDesc
{
	RsIndexBufferDesc();
	RsIndexBufferDesc( BcU32 NoofIndices );

	BcU32 NoofIndices_;
};

//////////////////////////////////////////////////////////////////////////
// RsIndexBuffer
class RsIndexBuffer:
	public RsResource
{
public:
	RsIndexBuffer( class RsContext* pContext );
	virtual ~RsIndexBuffer();
};

#endif
