#ifndef __BCVEC4DH__
#define __BCVEC4DH__

#include "Math/MaCPUVec4d.h"

typedef MaCPUVec4d MaVec4d;

template<>
struct ReTypeTraits< MaCPUVec4d >
{
	typedef MaCPUVec4d Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "class MaVec4d";
	}
};

#endif // __BCVEC4DH__
