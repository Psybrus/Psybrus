#pragma once

#include "Math/MaAABB.h"
#include "Math/MaPlane.h"

//////////////////////////////////////////////////////////////////////////
// MaFrustum
class MaFrustum
{
public:
	REFLECTION_DECLARE_BASIC( MaAABB );

	MaFrustum();
	MaFrustum( const MaMat4d& ClipTransform );

	/**
	 * Perform intersection test.
	 * @param SphereCentre Centre of sphere.
	 * @param Radius Radius of sphere.
	 * @return true if intersecting, false if not.
	 */
	bool intersect( const MaVec3d& SphereCentre, BcF32 Radius ) const;

private:
	std::array< MaPlane, 6 > Planes_;
};

