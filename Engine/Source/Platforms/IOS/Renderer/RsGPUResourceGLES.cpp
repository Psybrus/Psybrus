/**************************************************************************
*
* File:		RsGPUResourceGLES.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsGPUResourceGLES.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsGPUResourceGLES::RsGPUResourceGLES():
	Handle_( 0 ),
	pData_( NULL ),
	DataSize_( 0 ),
	DeleteData_( BcFalse )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsGPUResourceGLES::~RsGPUResourceGLES()
{
	// Delete our data if we've been told to.
	if( DeleteData_ == BcTrue )
	{
		delete [] (BcU8*)pData_;
	}
	
	//
	BcAssert( Handle_ == 0 );
}

