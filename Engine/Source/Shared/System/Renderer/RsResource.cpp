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
// RsResourceDataInputDesc
RsResourceDataInputDesc::RsResourceDataInputDesc( void* pData, BcU32 DataSize ):
	pData_( pData ),
	DataSize_( DataSize ),
	TargetOffset_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// RsResourceDataInputDesc
RsResourceDataInputDesc::RsResourceDataInputDesc( void* pData, BcU32 DataSize, BcU32 TargetOffset ):
	pData_( pData ),
	DataSize_( DataSize ),
	TargetOffset_( TargetOffset )
{

}

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