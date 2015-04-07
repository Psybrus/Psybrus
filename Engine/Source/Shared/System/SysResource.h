/**************************************************************************
*
* File:		SysResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		System resource.
*		Used to handling of low level hardware resources, and
*		dealing with systems that run on 1 or more threads.
*		Also has a 64-bit handle internally if required.
* 
**************************************************************************/

#ifndef __SYSRESOURCE_H__
#define __SYSRESOURCE_H__

#include "Base/BcTypes.h"
#include "System/SysFence.h"

////////////////////////////////////////////////////////////////////////////////
// SysResource
class SysResource
{
public:
	SysResource();
	virtual ~SysResource();

	/**
	 * Called by the system's thread to create the resource.
	 */
	virtual void						create();

	/**
	 * Called by the system's thread to update the resource.
	 */
	virtual void						update();

	/**
	 * Called on the main thread by the system before destroying the resource.
	 */
	virtual void						preDestroy();	

	/**
	 * Called on the system's thread to destroy the resource.
	 */
	virtual void						destroy();

public:
	template< typename _Ty >
	BcForceInline _Ty					getHandle() const
	{
		return (_Ty)Handle_;		
	}

	template< typename _Ty >
	BcForceInline void					setHandle( _Ty Handle )
	{
		Handle_ = (BcU64)Handle;		
	}

	BcBool								hasHandle() const
	{
		return Handle_ != 0;
	}

private:
	BcU64								Handle_;
};

#endif
