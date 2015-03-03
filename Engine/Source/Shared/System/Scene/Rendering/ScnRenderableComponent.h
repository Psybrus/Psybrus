/**************************************************************************
*
* File:		Rendering/ScnRenderableComponent.h
* Author:	Neil Richardson 
* Ver/Date:	28/12/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRenderableComponent_H__
#define __ScnRenderableComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnSpatialComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponent
class ScnRenderableComponent:
	public ScnSpatialComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, ScnSpatialComponent );
	DECLARE_VISITABLE( ScnRenderableComponent );

public:
	void initialise();
	virtual void initialise( const Json::Value& Object );
	virtual void update( BcF32 Tick );
	virtual void render( 
		class ScnViewComponent* pViewComponent, 
		class RsFrame* pFrame, 
		RsRenderSort Sort );
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	/**
	 * Is this renderable component lit?
	 */
	BcBool isLit() const;

private:
	BcU32 RenderMask_;			// Used to specify what kind of object it is for selectively rendering with certain views.
	BcBool IsLit_;				// Does this need to be lit?
};

#endif
