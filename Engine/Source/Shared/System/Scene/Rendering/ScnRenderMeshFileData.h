#pragma once

#include "Math/MaAABB.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderMeshFileData
struct ScnRenderMeshFileData
{
	RsTopologyType ToplogyType_ = RsTopologyType::TRIANGLE_LIST;
	BcU32 NoofVertices_ = 0;
	BcU32 VertexStride_ = 0;
	BcU32 NoofIndices_ = 0;
	BcU32 IndexStride_ = 0;
	BcU32 NoofVertexElements_ = 0;
	BcU32 NoofDraws_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// ScnRenderMeshDraw
struct ScnRenderMeshDraw
{
	BcU32 VertexOffset_ = 0;
	BcU32 NoofVertices_ = 0;
	BcU32 IndexOffset_ = 0;
	BcU32 NoofIndices_ = 0;
};

