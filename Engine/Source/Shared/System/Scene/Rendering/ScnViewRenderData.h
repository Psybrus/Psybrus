#pragma once

#include "Base/BcTypes.h"

#include "System/Renderer/RsRenderNode.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewRenderData
class ScnViewRenderData
{
public:
	ScnViewRenderData();
	virtual ~ScnViewRenderData();

	void setSortPassType( RsRenderSortPassType SortPassType ) { SortPassType_ = SortPassType; }
	RsRenderSortPassType getSortPassType() const { return SortPassType_; }

private:
	friend class ScnViewComponent;

	RsRenderSortPassType SortPassType_ = RsRenderSortPassType::INVALID;

};
