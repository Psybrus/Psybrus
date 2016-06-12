#pragma once 

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/Rendering/ScnTexture.h"

//////////////////////////////////////////////////////////////////////////
// ScnDeferredRendererVertex
struct ScnDeferredRendererVertex
{
	REFLECTION_DECLARE_BASIC( ScnDeferredRendererVertex);
	ScnDeferredRendererVertex();
	ScnDeferredRendererVertex( const MaVec4d& Position, const MaVec2d& UV, const MaVec2d& Screen );

	MaVec4d Position_;
	MaVec2d UV_;
	MaVec2d Screen_;
};

//////////////////////////////////////////////////////////////////////////
// ScnDeferredRendererComponent
class ScnDeferredRendererComponent:
	public ScnComponent,
	public ScnVisitor
{
public:
	REFLECTION_DECLARE_DERIVED( ScnDeferredRendererComponent, ScnComponent );

	ScnDeferredRendererComponent();
	virtual ~ScnDeferredRendererComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void visit( class ScnLightComponent* Component ) override;
	
protected:
	void recreateResources();

protected:
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;

	enum : size_t
	{
		TEX_GBUFFER_ALBEDO = 0,
		TEX_GBUFFER_MATERIAL,
		TEX_GBUFFER_NORMAL,
		TEX_GBUFFER_VELOCITY,
		TEX_GBUFFER_DEPTH,
		TEX_LIGHTING,

		TEX_MAX
	};

	std::array< ScnTextureRef, TEX_MAX > Textures_;

	RsFrameBufferUPtr GBuffer_;
	RsFrameBufferUPtr LightingBuffer_;

	ScnViewComponent* View_ = nullptr;

	std::array< ScnShaderRef, scnLT_MAX > Shaders_;
	std::vector< ScnLightComponent* > LightComponents_;

	RsRenderStateUPtr RenderState_;
	RsSamplerStateUPtr SamplerState_;
	RsBufferUPtr UniformBuffer_;
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;
};

