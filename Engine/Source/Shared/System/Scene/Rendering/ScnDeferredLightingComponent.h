#pragma once 

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/Rendering/ScnTexture.h"

//////////////////////////////////////////////////////////////////////////
// ScnDeferredLightingVertex
struct ScnDeferredLightingVertex
{
	REFLECTION_DECLARE_BASIC( ScnDeferredLightingVertex );
	ScnDeferredLightingVertex();
	ScnDeferredLightingVertex( const MaVec4d& Position, const MaVec2d& UV );

	MaVec4d Position_;
	MaVec2d UV_;
};

//////////////////////////////////////////////////////////////////////////
// ScnDeferredLightingComponent
class ScnDeferredLightingComponent:
	public ScnRenderableComponent,
	public ScnVisitor
{
public:
	REFLECTION_DECLARE_DERIVED( ScnDeferredLightingComponent, ScnRenderableComponent );

	ScnDeferredLightingComponent();
	virtual ~ScnDeferredLightingComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	MaAABB getAABB() const override;

	void visit( class ScnLightComponent* Component ) override;

	void render( ScnRenderContext & RenderContext ) override;
	
protected:
	void recreateResources();

protected:
	std::map< std::string, ScnTextureRef > Textures_;
	std::array< ScnShaderRef, scnLT_MAX > Shaders_;
	std::vector< ScnLightComponent* > LightComponents_;

	std::array< RsProgramBindingUPtr, scnLT_MAX > ProgramBindings_;

	RsRenderStateUPtr RenderState_;
	RsSamplerStateUPtr SamplerState_;
	RsBufferUPtr UniformBuffer_;
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;
};

