/**************************************************************************
*
* File:		RsComputeInterface.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCOMPUTEINTERFACE_H__
#define __RSCOMPUTEINTERFACE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsProgramBinding.h"

//////////////////////////////////////////////////////////////////////////
// RsComputeInterface
class RsComputeInterface
{
public:
	virtual ~RsComputeInterface(){};

	/**
	 * Dispatch compute.
	 * @param ProgramBinding Program binding.
	 * @param XGroups Number of X groups to dispatch.
	 * @param YGroups Number of Y groups to dispatch.
	 * @param ZGroups Number of Z groups to dispatch.
	 */
	virtual void dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups ) { BcBreakpoint; };
};

#endif
