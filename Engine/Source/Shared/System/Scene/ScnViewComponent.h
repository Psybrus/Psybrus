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
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnRenderTarget.h"
#include "System/Scene/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewComponentRef
typedef CsResourceRef< class ScnViewComponent > ScnViewComponentRef;
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
	DECLARE_VISITABLE( ScnViewComponent );

#if PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	void								initialise();
	void								initialise( BcReal X, BcReal Y, BcReal Width, BcReal Height, BcReal Near, BcReal Far, BcReal HorizontalFOV, BcReal VerticalFOV );
	virtual void						initialise( const Json::Value& Object );
	
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	void								setMaterialParameters( ScnMaterialComponentRef MaterialComponent );
	void								getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far );

	virtual void						bind( RsFrame* pFrame, RsRenderSort Sort );
	
	void								setRenderMask( BcU32 RenderMask );
	const BcU32							getRenderMask() const;

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:

	struct THeader
	{
		// Viewport. Values relative to the size of the client being rendered into.
		BcReal							X_;
		BcReal							Y_;
		BcReal							Width_;
		BcReal							Height_;

		// Perspective projection.
		BcReal							Near_;
		BcReal							Far_;
		BcReal							HorizontalFOV_;		// Used by default.
		BcReal							VerticalFOV_;		// Used if HorizontalFOV_ is 0.0.
	};
	
	THeader								Header_;
	
	ScnRenderTargetRef					RenderTarget_;
	BcMat4d								InverseViewMatrix_;
	RsViewport							Viewport_;

	BcU32								RenderMask_;		// Used to determine what objects should be rendered for this view.
};

#endif
