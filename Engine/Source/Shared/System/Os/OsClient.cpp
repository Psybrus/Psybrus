/**************************************************************************
*
* File:		OsClient.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Operating system client.
*		
*
*
* 
**************************************************************************/

#include "OsClient.h"

#include "OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
OsClient::OsClient()
{
	OsCore::pImpl()->registerClient( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsClient::~OsClient()
{
	OsCore::pImpl()->unregisterClient( this );
}
