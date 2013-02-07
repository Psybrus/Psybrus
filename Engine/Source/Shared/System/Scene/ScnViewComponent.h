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

#if PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	void								initialise();
	virtual void						initialise( const Json::Value& Object );

	void								setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;
	void								getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far ) const;
	BcVec2d								getScreenPosition( const BcVec3d& WorldPosition ) const;
	const RsViewport&					getViewport() const;

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
	BcMat4d								InverseViewMatrix_;

	// TODO: Remove this dependency, not really needed.
	RsViewport							Viewport_;

	ScnRenderTargetRef					RenderTarget_;
};

#endif
