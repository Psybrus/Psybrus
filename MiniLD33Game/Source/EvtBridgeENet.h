/**************************************************************************
*
* File:		EvtBridgeENet.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge over ENet.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTBRIDGEENET_H__
#define __EVTBRIDGEENET_H__

#include <enet/enet.h>

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// EvtBridgeENet
class EvtBridgeENet: 
	public EvtBridge
{
public:
	EvtBridgeENet( EvtPublisher* pPublisher );
	virtual ~EvtBridgeENet();

	BcBool connect( BcU32 ClientID, const BcChar* pAddress, BcU16 Port );
	BcBool update( BcReal Delta );

private:
	virtual void bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

private:
	void serviceHost( ENetHost* pHost );

private:
	// SERVER
	ENetAddress ServerAddress_;
	ENetHost* pServerHost_;

	// CLIENT.
	BcU32 ClientID_;
	ENetPeer* pPeer_;
	ENetHost* pClientHost_;

	BcBool IsConnected_;

	BcU32 NoofHosts_;
};

#endif
