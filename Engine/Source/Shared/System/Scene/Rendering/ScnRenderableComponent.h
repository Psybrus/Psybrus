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
		View_( pViewComponent ),
		ViewRenderData_( nullptr ),
		pFrame_( pFrame ),
		Sort_( Sort )
	{}

	class ScnViewComponent* View_;
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
	void destroyViewRenderData( class ScnComponent* Component, class ScnViewComponent* View, ScnViewRenderData* ViewRenderData ) override;
	void render( const ScnViewComponentRenderData* ComponentRenderDatas, BcU32 NoofComponents, class ScnRenderContext& RenderContext ) override;
	void getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents ) override;
	void getRenderMask( BcU32* OutRenderMasks, class ScnComponent** Components, BcU32 NooComponents ) override;

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
	
	virtual void render( ScnRenderContext& RenderContext );
	virtual MaAABB getAABB() const;

	void setRenderMask( BcU32 RenderMask );
	const BcU32 getRenderMask() const;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

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
	virtual void destroyViewRenderData( class ScnViewComponent* View, ScnViewRenderData* ViewRenderData );

private:
	/// Used to specify what kind of object it is for selectively rendering with certain views.
	BcU32 RenderMask_;
	/// Render permutation flags that this renderable supports.
	ScnShaderPermutationFlags RenderPermutations_;
	/// Sort pass flags that this renderable supports.
	RsRenderSortPassFlags Passes_;
};

#endif
