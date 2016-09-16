#pragma once

#include "Base/BcTypes.h"
#include "Math/MaMat4d.h"

namespace Editor
{
	void Init();

	/**
	 * Result from handle with interaction details.
	 */
	struct HandleResult 
	{
		bool WasClicked_ = false;
		BcU32 ButtonClicked_ = 0;
		BcU32 SelectedID_ = 0;
		MaVec3d WorldPosition_;
		MaVec2d ScreenPosition_;

		operator bool()
		{
			return WasClicked_;
		}
	};
	
	/**
	 * Setup handle in world.
	 * @param ID ID to associate with anchor.
	 * @param Name of anchor. Included as part of the ID. Must be persistent until next frame.
	 * @param Position Position in world.
	 * @return Handle result.
	 */
	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d Position, BcF32 Size = 8.0f );

	/**
	 * Setup handle in world between two points.
	 * @param ID ID to associate with anchor.
	 * @param Name of anchor. Included as part of the ID. Must be persistent until next frame.
	 * @param PointA Start position in world.
	 * @param PointB End position in world.
	 * @return Handle result.
	 */
	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d PointA, const MaVec3d PointB, BcF32 Size = 6.0f );

	/**
	 * Deselect currently selected handle.
	 */
	void DeselectHandle();
}
