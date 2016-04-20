#pragma once

//////////////////////////////////////////////////////////////////////////
// ScnViewComponentRenderData
struct ScnViewComponentRenderData
{
	class ScnComponent* Component_ = nullptr;
	class ScnViewRenderData* ViewRenderData_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnViewRenderInterface
class ScnViewRenderInterface
{
public:
	/**
	 * Create view render data.
	 * Should return a new @a ScnViewRenderData or derived type if rendering if required. nullptr if not.
	 * If overridden, no need to call this function from the override.
	 * @param Component Component to create view render data for.
	 * @param View View to create render data for.
	 */
	virtual class ScnViewRenderData* createViewRenderData( class ScnComponent* Component, class ScnViewComponent* View ) = 0;

	/**
	 * Destroy view render data.
	 * Will delete @a ViewRenderData by default. Optional to overload.
	 * If overridden, no need to call this function from the override.
	 * @param Component Component to destroy view render data for.
	 * @param View View that this render data is associated with.
	 * @param ViewRenderData View render data to destroy.
	 */
	virtual void destroyViewRenderData( class ScnComponent* Component, class ScnViewComponent* View, ScnViewRenderData* ViewRenderData ) = 0;

	/**
	 * Get AABB.
	 */
	virtual void getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents ) = 0;

	/**
	 * Get render mask.
	 */
	virtual void getRenderMask( BcU32* OutRenderMasks, class ScnComponent** Components, BcU32 NooComponents ) = 0;
	
	/**
	 * Do render.
	 */
	virtual void render( const ScnViewComponentRenderData* ComponentRenderData, BcU32 NoofComponents, class ScnRenderContext & RenderContext ) = 0;
};
