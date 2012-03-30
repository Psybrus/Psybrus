#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "GetTime.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"
#include "RakSleep.h"
#include "FullyConnectedMesh2.h"
#include "ConnectionGraph2.h"
#include <assert.h>
#include "SocketLayer.h"
#include "Kbhit.h"
#include "PacketLogger.h"
#include "BitStream.h"

using namespace RakNet;

RakNet::RakPeerInterface *rakPeer;

int main()
{
	FullyConnectedMesh2 fcm2;
	ConnectionGraph2 cg2;
	rakPeer=RakNet::RakPeerInterface::GetInstance();
	rakPeer->AttachPlugin(&fcm2);
	rakPeer->AttachPlugin(&cg2);
	fcm2.SetAutoparticipateConnections(true);
	RakNet::SocketDescriptor sd;
	sd.socketFamily=AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
	sd.port=60000;
	while (SocketLayer::IsPortInUse(sd.port, sd.hostAddress, sd.socketFamily)==true)
		sd.port++;
	StartupResult sr = rakPeer->Startup(8,&sd,1);
	RakAssert(sr==RAKNET_STARTED);
	rakPeer->SetMaximumIncomingConnections(8);
	rakPeer->SetTimeoutTime(1000,RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	printf("Our guid is %s\n", rakPeer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
	printf("Started on %s\n", rakPeer->GetMyBoundAddress().ToString(true));

//	PacketLogger packetLogger;
//	rakPeer->AttachPlugin(&packetLogger);
//	packetLogger.SetLogDirectMessages(false);

	bool quit=false;
	RakNet::Packet *packet;
	char ch;
	while (!quit)
	{
		for (packet = rakPeer->Receive(); packet; rakPeer->DeallocatePacket(packet), packet = rakPeer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf("ID_DISCONNECTION_NOTIFICATION\n");
				break;


			case ID_NEW_INCOMING_CONNECTION:
				// Somebody connected.  We have their IP now
				printf("ID_NEW_INCOMING_CONNECTION from %s. guid=%s.\n", packet->systemAddress.ToString(true), packet->guid.ToString());
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				// Somebody connected.  We have their IP now
				printf("ID_CONNECTION_REQUEST_ACCEPTED from %s. guid=%s.\n", packet->systemAddress.ToString(true), packet->guid.ToString());
				break;


			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf("ID_CONNECTION_LOST\n");
				break;

			case ID_ADVERTISE_SYSTEM:
				if (packet->guid!=rakPeer->GetMyGUID())
					rakPeer->Connect(packet->systemAddress.ToString(false), packet->systemAddress.GetPort(),0,0);
				break;

			case ID_FCM2_NEW_HOST:
				{
				if (packet->guid==rakPeer->GetMyGUID())
					printf("Got new host (ourselves)");
				else
					printf("Got new host %s, GUID=%s", packet->systemAddress.ToString(true), packet->guid.ToString());
					RakNet::BitStream bs(packet->data,packet->length,false);
					bs.IgnoreBytes(1);
					RakNetGUID oldHost;
					bs.Read(oldHost);
					// If the old host is different, then this message was due to losing connection to the host.
					if (oldHost!=packet->guid)
						printf(". Oldhost Guid=%s\n", oldHost.ToString());
					else
						printf("\n");
				}
				break;

// 			case ID_REMOTE_NEW_INCOMING_CONNECTION:
// 				{
// 					uint32_t count;
// 					RakNet::BitStream bsIn(packet->data, packet->length,false);
// 					bsIn.IgnoreBytes(1);
// 					bsIn.Read(count);
// 					SystemAddress sa;
// 					RakNetGUID guid;
// 					printf("ID_REMOTE_NEW_INCOMING_CONNECTION from %s\n", packet->systemAddress.ToString(true));
// 					for (uint32_t i=0; i < count; i++)
// 					{
// 						bsIn.Read(sa);
// 						bsIn.Read(guid);
// 
// 						printf("%s ", sa.ToString(true));
// 					}
// 					printf("\n");
// 				}
			}
		}

		if (kbhit())
		{
			ch=getch();
			if (ch==' ')
			{
				unsigned int participantList;
				fcm2.GetParticipantCount(&participantList);
				printf("participantList=%i\n",participantList);
			}
			if (ch=='q' || ch=='Q')
			{
				printf("Quitting.\n");
				quit=true;
			}
		}

		RakSleep(30);
		for (int i=0; i < 32; i++)
		{
			if (rakPeer->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS,0).GetPort()!=60000+i)
				rakPeer->AdvertiseSystem("255.255.255.255", 60000+i, 0,0,0);
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(rakPeer);
	return 0;
}
