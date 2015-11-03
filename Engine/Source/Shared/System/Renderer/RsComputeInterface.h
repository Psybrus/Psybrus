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

//////////////////////////////////////////////////////////////////////////
// RsDispatchBindings
struct RsDispatchBindings
{
	RsDispatchBindings():
		Buffers_( { nullptr } ),
		UniformBuffers_( { nullptr } )
	{}

	/// Buffers.
	std::array< class RsBuffer*, 32 > Buffers_;

	/// Uniform buffers.
	std::array< class RsBuffer*, 32 > UniformBuffers_;
};

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
	virtual void dispatchCompute( class RsProgram* Program, RsDispatchBindings& Bindings, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups ) { BcBreakpoint; };
};

#endif
