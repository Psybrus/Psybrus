#pragma once

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"


/**
 *	@class NsCore
 *	@brief Network System Core 
 */
class NsCore:
	public BcGlobal< NsCore >,
	public SysSystem
{
public:
	NsCore();
	virtual ~NsCore();

	/**
	 * Create network session.
	 */

};
