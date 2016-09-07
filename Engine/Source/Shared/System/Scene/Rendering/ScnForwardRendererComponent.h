#pragma once 

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnForwardRendererComponent
class ScnForwardRendererComponent:
	public ScnComponent,
	public ScnViewCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnForwardRendererComponent, ScnComponent );

	ScnForwardRendererComponent();
	virtual ~ScnForwardRendererComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	
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

	// ScnViewCallback
	void onViewDrawPreRender( ScnRenderContext& RenderContext ) override;
	void onViewDrawPostRender( ScnRenderContext& RenderContext ) override;

protected:
	BcBool Enabled_ = BcTrue;
	BcS32 Width_ = 0;
	BcS32 Height_ = 0;

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

	BcBool UseEnvironmentProbes_ = BcTrue;
	ScnTextureRef ReflectionCubemap_;

	ScnViewComponent* MainView_ = nullptr;

	RsFrameBuffer* ResolveTarget_ = nullptr;
};

