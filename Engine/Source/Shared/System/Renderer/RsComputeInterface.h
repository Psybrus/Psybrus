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
	RsDispatchBindings()
	{
		UniformBuffers_.fill( nullptr );
		SamplerStates_.fill( nullptr );
	}

	struct SRVSlot
	{
		RsShaderResourceType Type_ = RsShaderResourceType::INVALID;
		union
		{
			RsResource* Resource_ = nullptr;
			RsBuffer* Buffer_;
			RsTexture* Texture_;
		};
	};

	struct UAVSlot
	{
		RsUnorderedAccessType Type_ = RsUnorderedAccessType::INVALID;
		union
		{
			RsResource* Resource_ = nullptr;
			RsBuffer* Buffer_;
			RsTexture* Texture_;
		};
	};

	/// Slots.
	std::array< class RsBuffer*, 32 > UniformBuffers_;
	std::array< class RsSamplerState*, 32 > SamplerStates_;
	std::array< SRVSlot, 32 > ShaderResourceSlots_;
	std::array< UAVSlot, 8 > UnorderedAccessSlots_;
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
