#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsQueryHeapDesc
struct RsQueryHeapDesc
{
	RsQueryHeapDesc();
	RsQueryHeapDesc( RsQueryType QueryType, size_t NoofQueries );
	
	RsQueryType QueryType_ = RsQueryType::UNKNOWN;
	size_t NoofQueries_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsQueryHeap
class RsQueryHeap:
	public RsResource
{
public:
	RsQueryHeap( class RsContext* pContext, const RsQueryHeapDesc& QueryHeapDesc );
	virtual ~RsQueryHeap();
	
	const RsQueryHeapDesc& getDesc() const;

private:
	RsQueryHeapDesc QueryHeapDesc_;
};
