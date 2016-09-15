#pragma once

#include "Base/BcTypes.h"
#include "Math/MaMat4d.h"

namespace Editor
{
	void Init();
	
	/**
	 * Setup handle in world.
	 * @param ID ID to associate with anchor.
	 * @param Name of anchor. Included as part of the ID. Must be persistent until next frame.
	 * @param Position Position in world.
	 * @return true is anchor has been selected.
	 */
	bool Handle( BcU32 ID, const char* Name, const MaVec3d Position );
}
