#ifndef __BCVEC3DH__
#define __BCVEC3DH__

#include "Math/MaCPUVec3d.h"

typedef MaCPUVec3d MaVec3d;

template<>
struct ReTypeTraits< MaCPUVec3d >
{
	typedef MaCPUVec3d Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "class MaVec3d";
	}
};

#endif // __BCVEC3DH__
