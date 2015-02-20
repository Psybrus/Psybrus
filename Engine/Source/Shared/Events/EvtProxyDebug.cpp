/**************************************************************************
*
* File:		EvtProxyDebug.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy debugger. Logs all events which pass through.
*		
*		
*
* 
**************************************************************************/

#include "EvtProxyDebug.h"

#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
EvtProxyDebug::EvtProxyDebug( EvtPublisher* pPublisher, const BcName& Tag ):
	EvtProxy( pPublisher ),
	Tag_( Tag )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtProxyDebug::~EvtProxyDebug()
{
}

//////////////////////////////////////////////////////////////////////////
// proxy
//virtual
eEvtReturn EvtProxyDebug::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	BcChar PrefixA = ( ID >> 24 ) & 0xff;
	BcChar PrefixB = ( ID >> 16 ) & 0xff;
	BcU32 Group = ( ID >> 8 ) & 0xff;
	BcU32 Item = ( ID ) & 0xff;
		
	PSY_LOG( "EvtProxyDebug: Tag: %s, Publisher: %p, ID: %x, Prefixes: \"%c%c\": Group=%u Item=%u\n", (*Tag_).c_str(), pPublisher_, ID, PrefixA, PrefixB, Group, Item );

	// Always pass, we don't dispatch it.
	return evtRET_PASS;
}
