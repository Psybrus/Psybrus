/**************************************************************************
*
* File:		OsCoreImplAndroid.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Os/OsCoreImplAndroid.h"
#include "System/Os/OsClientAndroid.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplAndroid );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplAndroid::OsCoreImplAndroid()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplAndroid::~OsCoreImplAndroid()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplAndroid::open()
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplAndroid::update()
{
	OsCore::update();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplAndroid::close()
{

}
