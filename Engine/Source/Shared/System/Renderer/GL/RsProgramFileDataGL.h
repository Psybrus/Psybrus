#pragma once

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramParameterTypeGL
enum class RsProgramParameterStorageGL : BcU32
{
	UNIFORM = 0,
	UNIFORM_BLOCK,
	SAMPLER,
	SHADER_STORAGE_BUFFER,
	IMAGE
};

//////////////////////////////////////////////////////////////////////////
// RsProgramParameterTypeValueGL
union RsProgramParameterTypeValueGL
{
	struct
	{
		/// What type of storage the parameter uses.
		RsProgramParameterStorageGL Storage_ : 3;
		/// layout() coherent
		BcU32 Coherent_ : 1;
		/// layout() volatile
		BcU32 Volatile_ : 1;
		/// layout() restrict
		BcU32 Restrict_ : 1;
		/// layout() readonly
		BcU32 ReadOnly_ : 1;
		/// layout() writeonly
		BcU32 WriteOnly_ : 1;
		/// layout( binding )
		BcU32 Binding_ : 8;
		/// GL type (SAMPLER_1D, IMAGE_2D, etc)
		BcU32 Type_ : 16;
	};
	BcU32 Value_ = 0;
};
