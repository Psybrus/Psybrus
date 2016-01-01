/**************************************************************************
*
* File:		Rendering/ScnViewComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnViewComponent_H__
#define __ScnViewComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// ScnViewComponentRef
typedef ReObjectRef< class ScnViewComponent > ScnViewComponentRef;
typedef std::list< ScnViewComponentRef > ScnViewComponentList;
typedef ScnViewComponentList::iterator ScnViewComponentListIterator;
typedef ScnViewComponentList::const_iterator ScnViewComponentListConstIterator;
typedef std::map< std::string, ScnViewComponentRef > ScnViewComponentMap;
typedef ScnViewComponentMap::iterator ScnViewComponentMapIterator;
typedef ScnViewComponentMap::const_iterator ScnViewComponentMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnViewComponent
class ScnViewComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnViewComponent, ScnComponent );

	ScnViewComponent();
	virtual ~ScnViewComponent();
	
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Get view uniform buffer.
	 */
	class RsBuffer* getViewUniformBuffer(); 

	void setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;
	void getWorldPosition( const MaVec2d& ScreenPosition, MaVec3d& Near, MaVec3d& Far ) const;
	MaVec2d getScreenPosition( const MaVec3d& WorldPosition ) const;
	BcU32 getDepth( const MaVec3d& WorldPos ) const;

	BcBool intersect( const MaAABB& AABB ) const;
	BcBool hasRenderTarget() const;

	RsFrameBuffer* getFrameBuffer() const;
	const RsViewport& getViewport() const;

	virtual void bind( class RsFrame* pFrame, RsRenderSort Sort );
	
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	const ScnShaderPermutationFlags getRenderPermutation() const;
	const ScnShaderPermutationFlags getPassPermutations() const;

private:
	void recreateFrameBuffer();

	static void renderViews( const ScnComponentList& Components );	

private:
	// Viewport. Values relative to the size of the client being rendered into.
	BcF32 X_;
	BcF32 Y_;
	BcF32 Width_;
	BcF32 Height_;

	// Perspective projection.
	BcF32 Near_;
	BcF32 Far_;
	BcF32 HorizontalFOV_;		// Used by default.
	BcF32 VerticalFOV_;		// Used if HorizontalFOV_ is 0.0.

	RsColour ClearColour_;
	bool EnableClearColour_;
	bool EnableClearDepth_;
	bool EnableClearStencil_;	
	
	BcU32 RenderMask_;		// Used to determine what objects should be rendered for this view.
	
	// Permutation types.
	ScnShaderPermutationFlags RenderPermutation_;
	ScnShaderPermutationFlags PassPermutations_;

	// TODO: Remove this dependency, not really needed.
	RsViewport Viewport_;

	// Uniform block data.
	ScnShaderViewUniformBlockData ViewUniformBlock_;
	RsBufferUPtr ViewUniformBuffer_;

	// Used for culling.
	// TODO: Move into BcFrustum, or perhaps a BcConvexHull?
	MaPlane FrustumPlanes_[ 6 ];

	// Frame buffer + render target.
	std::vector< ScnTextureRef > RenderTarget_;
	ScnTextureRef DepthStencilTarget_;
	RsFrameBufferUPtr FrameBuffer_;
};

#endif
