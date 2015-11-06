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
#include "System/Renderer/RsComputeBindings.h"

//////////////////////////////////////////////////////////////////////////
// RsComputeInterface
class RsComputeInterface
{
public:
	virtual ~RsComputeInterface(){};

	/**
	 * Dispatch compute.
	 * @param Program Program to use.
	 * @param Bindings Resource bindings for dispatch.
	 * @param XGroups Number of X groups to dispatch.
	 * @param YGroups Number of Y groups to dispatch.
	 * @param ZGroups Number of Z groups to dispatch.
	 */
	virtual void dispatchCompute( class RsProgram* Program, RsComputeBindings& Bindings, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups ) { BcBreakpoint; };
};

#endif
