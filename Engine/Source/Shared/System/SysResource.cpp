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
	Handle_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// SysResource
//virtual
SysResource::~SysResource()
{

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

}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void SysResource::destroy()
{

}
