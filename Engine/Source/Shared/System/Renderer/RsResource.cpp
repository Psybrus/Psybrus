/**************************************************************************
*
* File:		RsResource.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer resource.
*		
*
*
* 
**************************************************************************/

#include "RsResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsResource
RsResource::RsResource():
	pData_( NULL ),
	DataSize_( 0 ),
	DeleteData_( BcFalse ),
	Handle_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// RsResource
//virtual
RsResource::~RsResource()
{
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsResource::create()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsResource::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// preDestroy
//virtual
void RsResource::preDestroy()
{
	// Delete our internal resource if we need to.
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}

	// NULL everything, as our parent object has likely been freed.
	pData_ = NULL;
	DataSize_ = 0;
	DeleteData_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsResource::destroy()
{
	
}
