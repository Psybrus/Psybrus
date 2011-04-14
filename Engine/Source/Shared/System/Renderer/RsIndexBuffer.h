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

#include "RsTypes.h"
#include "RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsIndexBuffer
class RsIndexBuffer:
	public RsResource
{
public:
	RsIndexBuffer();
	virtual ~RsIndexBuffer();
};

#endif
