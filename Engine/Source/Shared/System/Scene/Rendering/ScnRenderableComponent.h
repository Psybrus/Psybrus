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
#include "System/Scene/ScnCoreCallback.h"
#include "System/Scene/ScnSpatialComponent.h"

#include <unordered_map>

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
		ViewRenderData_( nullptr ),
		pFrame_( pFrame ),
		Sort_( Sort )
	{}

	class ScnViewComponent* pViewComponent_;
	class ScnViewRenderData* ViewRenderData_;
	class RsFrame* pFrame_;
	RsRenderSort Sort_;
};

//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponent
class ScnRenderableComponent:
	public ScnSpatialComponent,
	public ScnCoreCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnRenderableComponent, ScnSpatialComponent );
	DECLARE_VISITABLE( ScnRenderableComponent );

public:
	ScnRenderableComponent();
	virtual ~ScnRenderableComponent();

	/**
	 * Called when component is attached to the scene.
	 */
	void onAttachComponent( class ScnComponent* Component ) override;

	/**
	 * Called when component is detached from the scene.
	 */
	void onDetachComponent( class ScnComponent* Component ) override;

	/**
	 * Create view render data.
	 * Should return a new @a ScnViewRenderData or derived type if rendering if required. nullptr if not.
	 * If overridden, no need to call this function from the override.
	 */
	virtual class ScnViewRenderData* createViewRenderData( class ScnViewComponent* View );

	/**
	 * Destroy view render data.
	 * Will delete @a ViewRenderData by default. Optional to overload.
	 * If overridden, no need to call this function from the override.
	 */
	virtual void destroyViewRenderData( ScnViewRenderData* ViewRenderData );

	/**
	 * Get view render data.
	 * Will call @a createViewRenderData if it needs to.
	 */
	class ScnViewRenderData* getViewRenderData( class ScnViewComponent* ViewComponent );

	/**
	 * Reset all view render data associated with this renderable.
	 * Called if it needs to be recreated due to, for example, a material change.
	 * @param ViewComponent Reset all render data for this view only. nullptr for any view.
	 */
	void resetViewRenderData( ScnViewComponent* ViewComponent );
	
	virtual void render( ScnRenderContext & RenderContext );
	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Should this component render in @a View?
	 */
	bool shouldRenderInView( class ScnViewComponent* View ) const;

	/**
	 * Is this renderable component lit?
	 */
	bool isLit() const { return IsLit_; }

	/**
	 * Is this renderable component transparent?
	 */
	bool isTransparent() const { return IsTransparent_; }

	/**
	 * Set if renderable is lit.
	 */
	void setLit( bool Lit ) { IsLit_ = Lit; }

	/**
	 * Set if renderable is transparent.
	 */
	void setTransparent( bool Transparent ) { IsTransparent_ = Transparent; }

private:
	BcU32 RenderMask_;				/// Used to specify what kind of object it is for selectively rendering with certain views.
	bool IsLit_;					/// Does this need to be lit?
	bool IsTransparent_;			/// Is renderable transparent?

	// TODO: Look at a smarter way to store + look up ScnViewRenderData structures.
	std::unordered_map< class ScnViewComponent*, class ScnViewRenderData* > ViewRenderData_; /// View render data.
};

#endif
