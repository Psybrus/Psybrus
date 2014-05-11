/**************************************************************************
*
* File:		ScnViewComponent.h
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
#include "System/Renderer/RsUniformBuffer.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnShaderFileData.h"
#include "System/Scene/ScnRenderTarget.h"

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
	DECLARE_RESOURCE( ScnComponent, ScnViewComponent );

#if PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	void								initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();

	void								setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;
	void								getWorldPosition( const MaVec2d& ScreenPosition, MaVec3d& Near, MaVec3d& Far ) const;
	MaVec2d								getScreenPosition( const MaVec3d& WorldPosition ) const;
	BcU32								getDepth( const MaVec3d& WorldPos ) const;
	const RsViewport&					getViewport() const;

	BcBool								intersect( const MaAABB& AABB ) const;

	virtual void						bind( RsFrame* pFrame, RsRenderSort Sort );
	
	void								setRenderMask( BcU32 RenderMask );
	const BcU32							getRenderMask() const;
	
protected:

	// Viewport. Values relative to the size of the client being rendered into.
	BcF32								X_;
	BcF32								Y_;
	BcF32								Width_;
	BcF32								Height_;

	// Perspective projection.
	BcF32								Near_;
	BcF32								Far_;
	BcF32								HorizontalFOV_;		// Used by default.
	BcF32								VerticalFOV_;		// Used if HorizontalFOV_ is 0.0.
		
	BcU32								RenderMask_;		// Used to determine what objects should be rendered for this view.


	// TODO: Remove this dependency, not really needed.
	RsViewport							Viewport_;

	// Uniform block data.
	ScnShaderViewUniformBlockData		ViewUniformBlock_;
	RsUniformBuffer*					ViewUniformBuffer_;

	// Used for culling.
	// TODO: Move into BcFrustum, or perhaps a BcConvexHull?
	MaPlane								FrustumPlanes_[ 6 ];

	ScnRenderTargetRef					RenderTarget_;
};

#endif
