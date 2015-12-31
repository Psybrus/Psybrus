#pragma once

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewRenderData
class ScnViewRenderData
{
public:
	ScnViewRenderData();
	virtual ~ScnViewRenderData();

private:
	friend class ScnViewComponent;
};