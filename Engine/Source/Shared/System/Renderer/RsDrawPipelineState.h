#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/RsRenderState.h"

//////////////////////////////////////////////////////////////////////////
// RsGeometryBindingDesc
struct RsDrawPipelineStateDesc
{
public:
	RsDrawPipelineStateDesc();

	class RsVertexDeclaration* VertexDeclaration_;
	class RsProgram* Program_;
	class RsRenderState* RenderState_;
};

////////////////////////////////////////////////////////////////////////////////
// RsDrawPipelineState
class RsDrawPipelineState:
	public RsResource
{
public:
	RsDrawPipelineState( class RsContext* pContext, 
		const RsDrawPipelineStateDesc & Desc );

	virtual ~RsDrawPipelineState();

	const RsDrawPipelineStateDesc& getDesc() const;

private:
	RsDrawPipelineStateDesc Desc_;
};

