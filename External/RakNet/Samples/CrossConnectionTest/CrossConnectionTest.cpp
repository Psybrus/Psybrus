/// \file
/// \brief Tests connecting two peers at the same time with the internet simulator running.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "RakPeerInterface.h"

#include "PacketLogger.h"
#include "Rand.h"
#include "Kbhit.h"
#include <stdio.h> // Printf
#include "RakSleep.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "GetTime.h"

using namespace RakNet;

int main()
{
	printf("An internal test to test two peers connecting to each other\n");
	printf("at the same time.  This causes bugs so I fix them here\n");

	RakPeerInterface *rakPeer1, *rakPeer2;
	rakPeer1=RakPeerInterface::GetInstance();
	rakPeer2=RakPeerInterface::GetInstance();
	rakPeer1->SetMaximumIncomingConnections(8);
	rakPeer2->SetMaximumIncomingConnections(8);
	
	bool gotConnectionRequestAccepted[2];
	bool gotNewIncomingConnection[2];
	Packet *packet;
	SocketDescriptor sd1(60000,0);
	SocketDescriptor sd2(2000,0);
	unsigned short numSystems[2];

	while (1)
	{
		gotConnectionRequestAccepted[0]=false;
		gotConnectionRequestAccepted[1]=false;
		gotNewIncomingConnection[0]=false;
		gotNewIncomingConnection[1]=false;
		numSystems[0]=0;
		numSystems[1]=0;

		rakPeer1->Startup(1,&sd1, 1);
		rakPeer2->Startup(1,&sd2, 1);
		RakSleep(100);
		rakPeer1->Connect("127.0.0.1", 2000, 0, 0);
		rakPeer2->Connect("127.0.0.1", 60000, 0, 0);
		RakSleep(100);
		for (packet=rakPeer1->Receive(); packet; rakPeer1->DeallocatePacket(packet), packet=rakPeer1->Receive())
		{
			if (packet->data[0]==ID_NEW_INCOMING_CONNECTION)
				gotNewIncomingConnection[0]=true;
			else if (packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
				gotConnectionRequestAccepted[0]=true;
			else if (packet->data[0]==ID_CONNECTION_ATTEMPT_FAILED)
				printf("Error on rakPeer1, got ID_CONNECTION_ATTEMPT_FAILED\n");
		}
		for (packet=rakPeer2->Receive(); packet; rakPeer2->DeallocatePacket(packet), packet=rakPeer2->Receive())
		{
			if (packet->data[0]==ID_NEW_INCOMING_CONNECTION)
				gotNewIncomingConnection[1]=true;
			else if (packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
				gotConnectionRequestAccepted[1]=true;
			else if (packet->data[0]==ID_CONNECTION_ATTEMPT_FAILED)
				printf("Error on rakPeer2, got ID_CONNECTION_ATTEMPT_FAILED\n");
		}
		rakPeer1->GetConnectionList(0,&numSystems[0]);
		rakPeer2->GetConnectionList(0,&numSystems[1]);

		if (gotConnectionRequestAccepted[0]==true && gotConnectionRequestAccepted[1]==true)
		{
			printf("Test passed\n");
		}
		else if (numSystems[0]!=1 || numSystems[1]!=1)
		{
			printf("Test failed, system 1 has %i connections and system 2 has %i connections.\n", numSystems[0], numSystems[1]);
		}
		else if (gotConnectionRequestAccepted[0]==false && gotConnectionRequestAccepted[1]==false)
		{
			printf("Test failed, ID_CONNECTION_REQUEST_ACCEPTED is false for both instances\n");
		}
		else if (gotNewIncomingConnection[0]==true && gotNewIncomingConnection[1]==true)
		{
			printf("Test failed, ID_NEW_INCOMING_CONNECTION is true for both instances\n");
		}
		else if (gotConnectionRequestAccepted[0]==false && gotConnectionRequestAccepted[1]==false)
		{
			printf("Test failed, ID_NEW_INCOMING_CONNECTION is false for both instances\n");
		}
		else if (gotConnectionRequestAccepted[0]==true && gotNewIncomingConnection[1]==false)
		{
			printf("Test failed, ID_CONNECTION_REQUEST_ACCEPTED for first instance, but not ID_NEW_INCOMING_CONNECTION for second\n");
		}
		else if (gotConnectionRequestAccepted[1]==true && gotNewIncomingConnection[0]==false)
		{
			printf("Test failed, ID_CONNECTION_REQUEST_ACCEPTED for second instance, but not ID_NEW_INCOMING_CONNECTION for first\n");
		}
		else if ((int)gotConnectionRequestAccepted[0]+
			(int)gotConnectionRequestAccepted[1]!=1)
		{
			printf("Test failed, does not have exactly one instance of ID_CONNECTION_REQUEST_ACCEPTED\n");
		}
		else if ((int)gotNewIncomingConnection[0]+
			(int)gotNewIncomingConnection[1]!=1)
		{
			printf("Test failed, does not have exactly one instance of ID_NEW_INCOMING_CONNECTION\n");
		}
		else if ((int)gotConnectionRequestAccepted[0]+
			(int)gotConnectionRequestAccepted[1]+
			(int)gotNewIncomingConnection[0]+
			(int)gotNewIncomingConnection[1]!=2)
		{
			printf("Test failed, does not have exactly one instance of ID_CONNECTION_REQUEST_ACCEPTED and one instance of ID_NEW_INCOMING_CONNECTION\n");
		}
		else
			printf("Test passed\n");


		rakPeer1->Shutdown(0);
		rakPeer2->Shutdown(0);
		RakSleep(100);
	}


	return 0;
}
