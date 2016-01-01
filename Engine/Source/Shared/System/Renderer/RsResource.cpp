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
RsResource::RsResource( RsResourceType ResourceType, class RsContext* pContext ):
	ResourceType_( ResourceType ),
	pContext_( pContext )
#if PSY_DEBUG
	, DebugName_( "" )
#endif
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

////////////////////////////////////////////////////////////////////////////////
// setDebugName
void RsResource::setDebugName( const BcChar* DebugName )
{
#if PSY_DEBUG
	BcAssert( DebugName );
	DebugName_ = DebugName ? DebugName : "(No Debug Name)";
#endif
}

////////////////////////////////////////////////////////////////////////////////
// getDebugName
const BcChar* RsResource::getDebugName() const
{
#if PSY_DEBUG
	return DebugName_.c_str();
#else
	return "(No Debug Name)";
#endif
}