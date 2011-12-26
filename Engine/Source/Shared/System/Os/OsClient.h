/**************************************************************************
*
* File:		OsClient.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Operating system client.
*		
*
*
* 
**************************************************************************/

#ifndef __OSCLIENT_H__
#define __OSCLIENT_H__

#include "OsEvents.h"
#include "EvtPublisher.h"

//////////////////////////////////////////////////////////////////////////
// OsClient
class OsClient:
	public EvtPublisher
{
public:
	OsClient();
	virtual ~OsClient();

	

};

#endif