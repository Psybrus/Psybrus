#pragma once

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// Message types.
enum class NsReplicationMessageType
{
	/// Add object.
	ADD,
	/// Remove object.
	REMOVE,
	/// Object data.
	DATA,
};
