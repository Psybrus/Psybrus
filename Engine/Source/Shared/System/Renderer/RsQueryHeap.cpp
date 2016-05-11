#include "System/Renderer/RsQueryHeap.h"

//////////////////////////////////////////////////////////////////////////
// RsQueryHeapDesc
RsQueryHeapDesc::RsQueryHeapDesc()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsQueryHeapDesc::RsQueryHeapDesc( RsQueryType QueryType, size_t NoofQueries ):
	QueryType_( QueryType ),
	NoofQueries_( NoofQueries )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsQueryHeap::RsQueryHeap( class RsContext* pContext, const RsQueryHeapDesc& QueryHeapDesc ):
	RsResource( RsResourceType::BUFFER, pContext ),
	QueryHeapDesc_( QueryHeapDesc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual 
RsQueryHeap::~RsQueryHeap()
{
#if PSY_DEBUG
	QueryHeapDesc_ = RsQueryHeapDesc();
#endif
}
	
//////////////////////////////////////////////////////////////////////////
// getDesc
const RsQueryHeapDesc& RsQueryHeap::getDesc() const
{
	return QueryHeapDesc_;
}
