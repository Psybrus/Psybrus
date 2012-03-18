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

#include "System/SysResource.h"

////////////////////////////////////////////////////////////////////////////////
// SysResource
SysResource::SysResource():
	pData_( NULL ),
	DataSize_( 0 ),
	DeleteData_( BcFalse ),
	Handle_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// SysResource
//virtual
SysResource::~SysResource()
{
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void SysResource::create()
{

}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void SysResource::update()
{

}

////////////////////////////////////////////////////////////////////////////////
// preDestroy
//virtual
void SysResource::preDestroy()
{
	//
	wait();

	// Delete our internal resource if we need to.
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}

	// Null internal data (shouldn't need to at this point...but belt and braces!)
	pData_ = NULL;
	DataSize_ = 0;
	DeleteData_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void SysResource::destroy()
{

}

////////////////////////////////////////////////////////////////////////////////
// wait
void SysResource::wait()
{
	UpdateSyncFence_.wait( 0 );
}