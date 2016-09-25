#pragma once

#include "Base/BcTypes.h"
#include "Math/MaAABB.h"
#include "Math/MaMat4d.h"
#include "Math/MaVec3d.h"
#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Renderer/RsViewport.h"

#include "System/Debug/DsImGui.h"

//////////////////////////////////////////////////////////////////////////
// Debug Utilities
#if !PSY_PRODUCTION
namespace Debug
{
	class DrawCategory
	{
	public:
		DrawCategory( const char* Name );
		DrawCategory( BcU32 Category );
		~DrawCategory();

	private:
		BcU32 OldMask_;
	};

	struct Vertex
	{
		BcF32 X_, Y_, Z_, W_;
		BcU32 RGBA_;
	};

	void Init( size_t NoofVertices );
	void Shutdown();
	void DrawViewOverlay( ImDrawList* DrawList, const MaMat4d& View, const MaMat4d& Projection, const RsViewport& Viewport, RsContext* Context );
	void Render( class RsFrame* Frame, class RsFrameBuffer* FrameBuffer, const RsViewport& Viewport, class RsBuffer* ViewUniformBuffer, RsRenderSort Sort );
	void NextFrame();

	/**
	 * Get draw category mask. Can be used to more broadly filter draws.
	 */
	BcU32 GetDrawCategoryMask();

	/**
	 * Set draw category mask.
	 */
	void SetDrawCategoryMask( BcU32 CategoryMask );

	/**
	 * Get current category mask.
	 */
	BcU32 GetCurrentDrawCategoryMask();

	/**
	 * Set current category mask.
	 */
	void SetCurrentDrawCategoryMask( BcU32 Mask );

	/**
	 * Get category mask by name.
	 * Will create one if missing, will return 0 if unable to create more.
	 */
	BcU32 GetDrawCategoryMask( const char* Name );

	/**
	 * Get draw categories.
	 * @param OutCategoryNames Output category names.
	 * @param OutCategoryMasks Output category masks.
	 * @param MaxCategories Maximum number of categories to receive.
	 * @return Number of categories filled.
	 */
	size_t GetDrawCategories( const char** OutCategoryNames, BcU32* OutCategoryMasks, size_t MaxCategories );


	bool CanDraw( const char* CategoryName );
	bool CanDraw( BcU32 CategoryMask );
	void DrawLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawLines( const MaVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawMatrix( const MaMat4d& Matrix, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawGrid( const MaVec3d& Position, const MaVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer = 0 );
	void DrawEllipsoid( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawCircle( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawAABB( const MaAABB& AABB, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawShadowedText( const MaVec3d& WorldPosition, const RsColour& Colour, const char* Text, ... );


}
#endif
