/**************************************************************************
*
* File:		EvtBridgeRakNet.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge over RakNet.
*		
*		
*
* 
**************************************************************************/

#ifndef __EvtBridgeRakNet_H__
#define __EvtBridgeRakNet_H__

#include "RakPeerInterface.h"
#include "NatPunchthroughClient.h"

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// EvtBridgeRakNet
class EvtBridgeRakNet: 
	public EvtBridge
{
public:
	EvtBridgeRakNet( EvtPublisher* pPublisher );
	virtual ~EvtBridgeRakNet();

	BcBool connect( BcU32 ClientID, BcU32 RemoteAddress, BcU16 RemotePort, BcU16 LocalPort, int ServerSocketFileDesc );
	BcBool connectNAT( BcU32 ClientID, BcU64 RemoteGUID );
	BcBool update( BcReal Delta );

private:
	virtual void bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

private:
	BcU32 ClientID_;

	RakNet::RakPeerInterface* pPeer_;
	RakNet::NatPunchthroughClient* pNatPunchthroughClient_;

	RakNet::SocketDescriptor SocketDesc_;
};

#endif
