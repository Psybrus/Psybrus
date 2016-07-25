#pragma once 

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnDeferredRendererVertex
struct ScnDeferredRendererVertex
{
	REFLECTION_DECLARE_BASIC( ScnDeferredRendererVertex );
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
	public ScnVisitor,
	public ScnViewCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnDeferredRendererComponent, ScnComponent );

	ScnDeferredRendererComponent();
	virtual ~ScnDeferredRendererComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void visit( class ScnLightComponent* Component ) override;
	
	/**
	 * Render to specified frame.
	 * @param Frame Frame to render into.
	 * @param ResolveTarget Target to resolve to. nullptr for backbuffer.
	 * @param Sort Sort key.
	 */
	void render( RsFrame* Frame, RsFrameBuffer* ResolveTarget, RsRenderSort Sort );

	/**
	 * Set projection parameters.
	 */
	void setProjectionParams( BcF32 Near, BcF32 Far, BcF32 HorizonalFOV, BcF32 VerticalFOV );

	BcS32 getWidth() const { return Width_; }
	BcS32 getHeight() const { return Height_; }
	
protected:
	void recreateResources();

	static void setupQuad( class RsContext* Context, 
		RsVertexDeclaration* VertexDeclaration,
		RsBuffer* VertexBuffer,
		const MaVec2d& MinPos, 
		const MaVec2d& MaxPos, 
		const MaVec2d& UVSize );

	void renderLights( ScnRenderContext& RenderContext );
	void renderReflection( ScnRenderContext& RenderContext );
	void downsampleHDR( ScnRenderContext& RenderContext );
	void renderResolve( ScnRenderContext& RenderContext );

	// ScnViewCallback
	void onViewDrawPreRender( ScnRenderContext& RenderContext ) override;
	void onViewDrawPostRender( ScnRenderContext& RenderContext ) override;

protected:
	BcBool Enabled_ = BcTrue;
	BcS32 Width_ = 0;
	BcS32 Height_ = 0;
	std::array< ScnShaderRef, scnLT_MAX > LightShaders_;
	ScnShaderRef LuminanceComputeShader_;
	ScnShaderRef LuminanceTransferComputeShader_;
	ScnShaderRef DownsampleComputeShader_;
	ScnShaderRef ReflectionShader_;
	ScnShaderRef ResolveShader_;

	// Final resolve to the backbuffer (or other target)
	BcF32 ResolveX_ = 0.0f;
	BcF32 ResolveY_ = 0.0f;
	BcF32 ResolveW_ = 1.0f;
	BcF32 ResolveH_ = 1.0f;

	// Perspective projection.
	BcF32 Near_ = 0.1f;
	BcF32 Far_ = 1000.0f;
	BcF32 HorizontalFOV_ = BcPI * 0.25f;
	BcF32 VerticalFOV_ = 0.0f;

	// Post process uniforms.
	ScnShaderToneMappingUniformBlockData ToneMappingUniformBlock_;

	enum : size_t
	{
		TEX_GBUFFER_ALBEDO = 0,
		TEX_GBUFFER_MATERIAL,
		TEX_GBUFFER_NORMAL,
		TEX_GBUFFER_VELOCITY,
		TEX_GBUFFER_DEPTH,
		TEX_HDR,
		TEX_LUMINANCE,
		TEX_LUMINANCE2,
		
		TEX_MAX
	};

	std::array< ScnTextureRef, TEX_MAX > Textures_;

	BcBool UseEnvironmentProbes_ = BcTrue;
	ScnTextureRef ReflectionCubemap_;

	ScnViewComponent* OpaqueView_ = nullptr;
	ScnViewComponent* TransparentView_ = nullptr;
	ScnViewComponent* OverlayView_ = nullptr;

	std::vector< ScnLightComponent* > LightComponents_;

	std::array< RsProgramBindingUPtr, scnLT_MAX > LightProgramBindings_;
	RsProgramBindingUPtr ReflectionProgramBinding_;
	RsProgramBindingUPtr ResolveProgramBinding_;

	RsBufferUPtr ToneMappingUniformBuffer_;

	RsRenderStateUPtr AdditiveRenderState_;
	RsRenderStateUPtr ResolveRenderState_;
	RsSamplerStateUPtr NearestSamplerState_;
	RsSamplerStateUPtr SamplerState_;
	RsBufferUPtr UniformBuffer_;
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;
	RsFrameBuffer* ResolveTarget_ = nullptr;
};

