/**************************************************************************
*
* File:		SsResource.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Sound resource.
*		
*
*
* 
**************************************************************************/

#include "SsResource.h"

////////////////////////////////////////////////////////////////////////////////
// SsResource
SsResource::SsResource():
	pData_( NULL ),
	DataSize_( 0 ),
	DeleteData_( BcFalse ),
	Handle_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// SsResource
//virtual
SsResource::~SsResource()
{
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void SsResource::create()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void SsResource::update()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// preDestroy
//virtual
void SsResource::preDestroy()
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
void SsResource::destroy()
{
	
}
