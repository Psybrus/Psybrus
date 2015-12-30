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
// ScnRenderContext
class ScnRenderContext
{
public:
	ScnRenderContext( 
			class ScnViewComponent* pViewComponent,
			class RsFrame* pFrame,
			RsRenderSort Sort ):
		pViewComponent_( pViewComponent ),
		pFrame_( pFrame ),
		Sort_( Sort )
	{}

	class ScnViewComponent* pViewComponent_;
	class RsFrame* pFrame_;
	RsRenderSort Sort_;
};

//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponent
class ScnRenderableComponent:
	public ScnSpatialComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnRenderableComponent, ScnSpatialComponent );
	DECLARE_VISITABLE( ScnRenderableComponent );

public:
	ScnRenderableComponent();
	virtual ~ScnRenderableComponent();

	virtual class ScnViewRenderData* createViewRenderData( class ScnViewComponent* View );
	virtual void destroyViewRenderData( class ScnViewComponent* View, class ScnViewRenderData* ViewRenderData );

	void setViewRenderDataDirty();
	BcU32 getViewRenderDataVersion() const;
	
	virtual void render( ScnRenderContext & RenderContext );
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Is this renderable component lit?
	 */
	bool isLit() const;

private:
	BcU32 RenderMask_;				/// Used to specify what kind of object it is for selectively rendering with certain views.
	bool IsLit_;					/// Does this need to be lit?
	BcU32 ViewRenderDataVersion_;	/// Version of view render data. Used to recreate when dirtied.
};

#endif
