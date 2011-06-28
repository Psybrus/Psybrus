/**************************************************************************
*
* File:		SsResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Sound resource.
*		
*
*
* 
**************************************************************************/

#ifndef __SSRESOURCE_H__
#define __SSRESOURCE_H__

#include "SsTypes.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
class SsResource
{
public:
	SsResource();
	virtual ~SsResource();
	
	virtual void						create();
	virtual void						update();
	virtual void						preDestroy();	
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

protected:
	void*								pData_;
	BcU32								DataSize_;
	BcBool								DeleteData_;	

private:
	BcU64								Handle_;
};

#endif
