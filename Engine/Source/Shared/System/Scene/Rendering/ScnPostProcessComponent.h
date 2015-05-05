#pragma once 

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/Rendering/ScnTexture.h"

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessVertex
struct ScnPostProcessVertex
{
	REFLECTION_DECLARE_BASIC( ScnPostProcessVertex );
	ScnPostProcessVertex();
	ScnPostProcessVertex( const MaVec4d& Position, const MaVec2d& UV );

	MaVec4d Position_;
	MaVec2d UV_;
};

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessNode
struct ScnPostProcessNode
{
	REFLECTION_DECLARE_BASIC( ScnPostProcessNode );
	ScnPostProcessNode();

	std::map< std::string, ScnTextureRef > InputTextures_;
	std::map< std::string, RsSamplerStateDesc > InputSamplers_;
	std::map< BcU32, ScnTextureRef > OutputTextures_;

	class ScnShader* Shader_ = nullptr;

	RsRenderStateDesc RenderState_;
};

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessComponent
class ScnPostProcessComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPostProcessComponent, ScnRenderableComponent );

	ScnPostProcessComponent();
	virtual ~ScnPostProcessComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	MaAABB getAABB() const override;

	void render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort ) override;
	
protected:
	void recreateResources();

protected:
	ScnTextureRef Input_;
	ScnTextureRef Output_;
	std::vector< ScnPostProcessNode > Nodes_;
	std::vector< RsFrameBufferUPtr > FrameBuffers_;
	std::vector< RsRenderStateUPtr > RenderStates_;
	std::map< std::string, RsSamplerStateUPtr > SamplerStates_;

	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;

	SysFence RenderFence_;
};

