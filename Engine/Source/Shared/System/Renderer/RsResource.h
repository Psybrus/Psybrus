/**************************************************************************
*
* File:		RsResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer resource.
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCE_H__
#define __RSRESOURCE_H__

#include "RsTypes.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
class RsResource
{
public:
	RsResource();
	virtual ~RsResource();
	
	virtual void						create();
	virtual void						update();
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
