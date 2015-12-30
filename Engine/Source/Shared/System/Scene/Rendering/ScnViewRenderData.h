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

	class ScnRenderableComponent* Parent_ = nullptr;
	BcU32 Version_ = 0;
};