#ifndef __BCVEC2DH__
#define __BCVEC2DH__

#include "Math/MaCPUVec2d.h"

typedef MaCPUVec2d MaVec2d;

template<>
struct ReTypeTraits< MaCPUVec2d >
{
	typedef MaCPUVec2d Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "class MaVec2d";
	}
};

#endif // __BCVEC2DH__
