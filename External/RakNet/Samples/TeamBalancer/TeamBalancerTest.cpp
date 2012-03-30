#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "GetTime.h"
#include "Rand.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"

#include "TeamBalancer.h"
#include "FullyConnectedMesh2.h"
#include "ConnectionGraph2.h"
#include "TeamBalancer.h"
#include "Kbhit.h"
#include "RakSleep.h"
#include "RakNetTypes.h"

static const int NUM_PEERS=4;
RakNet::RakPeerInterface *rakPeer[NUM_PEERS];
RakNet::FullyConnectedMesh2 fullyConnectedMeshPlugin[NUM_PEERS];
RakNet::ConnectionGraph2 connectionGraphPlugin[NUM_PEERS];
RakNet::TeamBalancer teamBalancerPlugin[NUM_PEERS];
void GetTeams(int &team0, int &team1);
void Wait(void);

using namespace RakNet;

int main(void)
{
	int i;

	for (i=0; i < NUM_PEERS; i++)
		rakPeer[i]=RakNet::RakPeerInterface::GetInstance();

	printf("This project tests and demonstrates the team balancer mesh plugin.\n");
	printf("It uses FullyConnectedMesh2 to test in a peer to peer enviroment");
	printf("Difficulty: Intermediate\n\n");

	int peerIndex;

	// Initialize the message handlers
	for (peerIndex=0; peerIndex < NUM_PEERS; peerIndex++)
	{
//		fullyConnectedMeshPlugin[peerIndex].Startup(0,0);
		rakPeer[peerIndex]->AttachPlugin(&fullyConnectedMeshPlugin[peerIndex]);
		// The fully connected mesh relies on the connection graph plugin also being attached
		rakPeer[peerIndex]->AttachPlugin(&connectionGraphPlugin[peerIndex]);
		rakPeer[peerIndex]->AttachPlugin(&teamBalancerPlugin[peerIndex]);
		rakPeer[peerIndex]->SetMaximumIncomingConnections(NUM_PEERS);
	}

	// Initialize the peers
	for (peerIndex=0; peerIndex < NUM_PEERS; peerIndex++)
	{
		RakNet::SocketDescriptor socketDescriptor(60000+peerIndex,0);
		rakPeer[peerIndex]->Startup(NUM_PEERS, &socketDescriptor, 1);
	}

	// Give the threads time to properly start
	RakSleep(200);

	printf("Peers initialized. ");
	printf("Connecting each peer to the prior peer\n");
	
	// Connect each peer to the prior peer
	for (peerIndex=1; peerIndex < NUM_PEERS; peerIndex++)
	{
        rakPeer[peerIndex]->Connect("127.0.0.1", 60000+peerIndex-1, 0, 0);
	}


	RakSleep(200);

	RakNet::RakNetGUID hostGuid = fullyConnectedMeshPlugin[0].GetHostSystem();
	printf("Host is %s\n", hostGuid.ToString());

	unsigned int hostIndex;
	for (peerIndex=0; peerIndex < NUM_PEERS; peerIndex++)
	{
		if (hostGuid==rakPeer[peerIndex]->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS))
			hostIndex=peerIndex;
		teamBalancerPlugin[peerIndex].SetTeamSizeLimit(0, NUM_PEERS/2);
		teamBalancerPlugin[peerIndex].SetTeamSizeLimit(1, NUM_PEERS/2);
		RakAssert(teamBalancerPlugin[peerIndex].GetMyTeam(UNASSIGNED_NETWORK_ID)==UNASSIGNED_TEAM_ID);
	}

	int team0,team1;
	// system 0 requests team 0
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==0);

	// system 1 requests team 0 - check both on team 0
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==2);
	RakAssert(team1==0);

	// system 1 requests team 1 - check evenly balanced
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 1);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);

	// system 0 requests team 1 - check both on team 1
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 1);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==0);
	RakAssert(team1==2);

	// host sets autobalances team - check evenly balanced
	teamBalancerPlugin[hostIndex].SetForceEvenTeams(true);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);

	// system 1 requests opposite team - check evenly balanced
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, ! teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID));
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);
	RakNet::TeamId team1OldTeam = teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID);

	// system 0 requests opposite team - check evenly balanced and swapped
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, ! teamBalancerPlugin[0].GetMyTeam(UNASSIGNED_NETWORK_ID));
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);
	RakAssert(teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID)!=team1OldTeam);

	// system 2 requests team 0 - check that two systems on team 0, one system on team 1
	teamBalancerPlugin[2].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==2);
	RakAssert(team1==1);

	// system 3 requests team 0 - check that two systems on team 0, two systems on team 1
	teamBalancerPlugin[3].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==2);
	RakAssert(team1==2);

	// Unset autobalance teams - check that two systems on team 0, two systems on team 1
	teamBalancerPlugin[hostIndex].SetForceEvenTeams(false);

	// All systems set team 0, check same as before.
	RakNet::TeamId t0=teamBalancerPlugin[0].GetMyTeam(UNASSIGNED_NETWORK_ID);
	RakNet::TeamId t1=teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID);
	RakNet::TeamId t2=teamBalancerPlugin[2].GetMyTeam(UNASSIGNED_NETWORK_ID);
	RakNet::TeamId t3=teamBalancerPlugin[3].GetMyTeam(UNASSIGNED_NETWORK_ID);
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	teamBalancerPlugin[2].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	teamBalancerPlugin[3].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	RakAssert(t0==teamBalancerPlugin[0].GetMyTeam(UNASSIGNED_NETWORK_ID));
	RakAssert(t1==teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID));
	RakAssert(t2==teamBalancerPlugin[2].GetMyTeam(UNASSIGNED_NETWORK_ID));
	RakAssert(t3==teamBalancerPlugin[3].GetMyTeam(UNASSIGNED_NETWORK_ID));


	// All systems unset their teams
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, UNASSIGNED_TEAM_ID);
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, UNASSIGNED_TEAM_ID);
	teamBalancerPlugin[2].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, UNASSIGNED_TEAM_ID);
	teamBalancerPlugin[3].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, UNASSIGNED_TEAM_ID);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==0);
	RakAssert(team1==0);


	// system 0 requests team 0 - check
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==0);

	// system 0 requests team 1 - check
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 1);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==0);
	RakAssert(team1==1);

	// system 0 requests team 0 - check
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==0);

	// Set autobalance teams
	teamBalancerPlugin[hostIndex].SetForceEvenTeams(true);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==0);

	// system 1 requests team 0 - check evenly balanced
	teamBalancerPlugin[1].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);

	// Lock teams
	teamBalancerPlugin[hostIndex].SetLockTeams(true);

	// System 1 requests team 1 - would normally swap, but teams are locked
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 1);
	Wait();

	// Unset autobalance teams - check evenly balanced
	teamBalancerPlugin[hostIndex].SetForceEvenTeams(false);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);

	t0=teamBalancerPlugin[0].GetMyTeam(UNASSIGNED_NETWORK_ID);
	t1=teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID);

	// Unlock teams - check swapped
	teamBalancerPlugin[hostIndex].SetLockTeams(false);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);
	RakAssert(t0!=teamBalancerPlugin[0].GetMyTeam(UNASSIGNED_NETWORK_ID));
	RakAssert(t1!=teamBalancerPlugin[1].GetMyTeam(UNASSIGNED_NETWORK_ID));

	// Lock teams
	teamBalancerPlugin[hostIndex].SetLockTeams(true);

	// system 0 requests team 0 - check evenly balanced
	teamBalancerPlugin[0].RequestSpecificTeam(UNASSIGNED_NETWORK_ID, 0);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==1);
	RakAssert(team1==1);

	// Unlock teams - check both on team 0
	teamBalancerPlugin[hostIndex].SetLockTeams(false);
	Wait();
	GetTeams(team0,team1);
	RakAssert(team0==2);
	RakAssert(team1==0);

	for (i=0; i < NUM_PEERS; i++)
		RakNet::RakPeerInterface::DestroyInstance(rakPeer[i]);

	return 1;
}

void GetTeams(int &team0, int &team1)
{
	team0=0;
	team1=0;

	int peerIndex;
	for (peerIndex=0; peerIndex < NUM_PEERS; peerIndex++)
	{
		if (teamBalancerPlugin[peerIndex].GetMyTeam(UNASSIGNED_NETWORK_ID)==0)
			team0++;
		else if (teamBalancerPlugin[peerIndex].GetMyTeam(UNASSIGNED_NETWORK_ID)==1)
			team1++;
	}
}

void Wait(void)
{
	for (int count=0; count < 3; count++)
	{
		RakSleep(50);
		RakNet::Packet *packet;
		for (unsigned int i=0; i < NUM_PEERS; i++)
		{
			for (packet=rakPeer[i]->Receive(); packet; rakPeer[i]->DeallocatePacket(packet), packet=rakPeer[i]->Receive())
			{
				if (packet->data[0]==ID_TEAM_BALANCER_REQUESTED_TEAM_CHANGE_PENDING)
				{
					printf("ID_TEAM_BALANCER_REQUESTED_TEAM_CHANGE_PENDING for team %i for index %i\n",packet->data[1],i);
				}
				else if (packet->data[0]==ID_TEAM_BALANCER_TEAMS_LOCKED)
				{
					printf("ID_TEAM_BALANCER_TEAMS_LOCKED for team %i for index %i\n",packet->data[1],i);
				}
				else if (packet->data[0]==ID_TEAM_BALANCER_TEAM_ASSIGNED)
				{
					printf("ID_TEAM_BALANCER_TEAM_ASSIGNED for team %i for index %i\n",packet->data[1],i);
				}
			}
		}
	}
}

