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
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCoreCallback.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewRenderInterface.h"

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
// ScnRenderableProcessor
class ScnRenderableProcessor : 
	public BcGlobal< ScnRenderableProcessor >,
	public ScnComponentProcessor,
	public ScnViewRenderInterface
{
public:
	ScnRenderableProcessor();
	virtual ~ScnRenderableProcessor();

	/// ScnComponentProcessor
	void initialise() override;
	void shutdown() override;

	/// ScnViewRenderInterface
	class ScnViewRenderData* createViewRenderData( class ScnComponent* Component, class ScnViewComponent* View ) override;
	void destroyViewRenderData( class ScnComponent* Component, ScnViewRenderData* ViewRenderData ) override;
	void render( const ScnViewComponentRenderData* ComponentRenderDatas, BcU32 NoofComponents, class ScnRenderContext & RenderContext ) override;
	void getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents ) override;

private:
};


//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponent
class ScnRenderableComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnRenderableComponent, ScnComponent );

public:
	ScnRenderableComponent();
	virtual ~ScnRenderableComponent();

	/**
	 * Reset all view render data associated with this renderable.
	 * Called if it needs to be recreated due to, for example, a material change.
	 * @param ViewComponent Reset all render data for this view only. nullptr for any view.
	 */
	void resetViewRenderData( ScnViewComponent* ViewComponent );
	
	virtual void render( ScnRenderContext & RenderContext );
	virtual MaAABB getAABB() const;

	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Get sort pass type.
	 * This will get the highest matching type of sort pass for view.
	 * I.e. if view + renderable both have OPAQUE + TRANSPARENT, then TRANSPARENT is returned.
	 * @param View View to get sort pass type for.
	 */
	RsRenderSortPassType getSortPassType( class ScnViewComponent* View ) const;

	/**
	 * Is this renderable component lit?
	 */
	bool isLit() const { return IsLit_; }

	/**
	 * Set if renderable is lit.
	 */
	void setLit( bool Lit ) { IsLit_ = Lit; }

	/**
	 * Set render permutation flags.
	 */
	void setRenderPermutations( ScnShaderPermutationFlags RenderPermutations ) { RenderPermutations_ = RenderPermutations & ScnShaderPermutationFlags::RENDER_ALL; }
	
	/**
	 * Get render permutation flags.
	 */
	ScnShaderPermutationFlags getRenderPermutations() const { return RenderPermutations_ & ScnShaderPermutationFlags::RENDER_ALL; }

	/**
	 * Set sort pass flags.
	 */
	void setPasses( RsRenderSortPassFlags Passes ) { Passes_ = Passes; }

	/**
	 * Get sort pass flags.
	 */
	RsRenderSortPassFlags getPasses() const { return Passes_; }

protected:
	friend class ScnRenderableProcessor;

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

private:
	/// Used to specify what kind of object it is for selectively rendering with certain views.
	BcU32 RenderMask_;
	/// Does this need to be lit?
	bool IsLit_;
	/// Render permutation flags that this renderable supports.
	ScnShaderPermutationFlags RenderPermutations_;
	/// Sort pass flags that this renderable supports.
	RsRenderSortPassFlags Passes_;
};

#endif
