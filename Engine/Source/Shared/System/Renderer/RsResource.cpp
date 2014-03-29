/**************************************************************************
*
* File:		RsResource.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer resource.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
RsResource::RsResource( class RsContext* pContext ):
	pContext_( pContext )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// RsResource
//virtual
RsResource::~RsResource()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// getContext
class RsContext* RsResource::getContext()
{
	return pContext_;
}