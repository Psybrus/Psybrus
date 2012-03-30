
#include "RakPeerInterface.h"
#include "FileOperations.h"
#include "RakMemoryOverride.h"
#include "ConsoleServer.h"
#include "LogCommandParser.h"
#include "RakNetCommandParser.h"
#include "PacketLogger.h"
#include "DS_List.h"
#include "SocketLayer.h"
#include "RakSleep.h"

using namespace RakNet;

void* MyMalloc (size_t size)
{
	return malloc(size);
}

void* MyRealloc (void *p, size_t size)
{
	return realloc(p,size);
}

void MyFree (void *p)
{
	free(p);
}

// This project is used to test the DLL system to make sure necessary classes are exported
int main()
{
	// Just test allocation and deallocation across the DLL.  If it crashes it failed, otherwise it worked.
	ConsoleServer* a=ConsoleServer::GetInstance( );
	LogCommandParser* c=LogCommandParser::GetInstance( );
	PacketLogger* d=PacketLogger::GetInstance( );
	RakNetCommandParser* e=RakNetCommandParser::GetInstance( );
	RakPeerInterface * f=RakPeerInterface::GetInstance( );
	SystemAddress sa = UNASSIGNED_SYSTEM_ADDRESS;

	SocketDescriptor sd(5555,0);
	if(f->Startup(32,&sd,1) != RAKNET_STARTED) {		
			printf("NetworkNode::startup(): failed to start server\n");
			return 0;
		}
	f->SetMaximumIncomingConnections(32);
	
	DataStructures::List<RakNetSmartPtr<RakNetSocket> > sockets;
	f->GetSockets(sockets);
	f->ReleaseSockets(sockets);

	// See RakMemoryOverride.h
	SetMalloc(MyMalloc);
	SetRealloc(MyRealloc);
	SetFree(MyFree);

	char *cArray = RakNet::OP_NEW_ARRAY<char>(10,_FILE_AND_LINE_);
	RakNet::OP_DELETE_ARRAY(cArray,_FILE_AND_LINE_);

	DataStructures::List<int> intList;
	intList.Push(5, _FILE_AND_LINE_ );
	
	f->GetMTUSize(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	SystemAddress p1;
	SystemAddress p2;
	p1=p2;

	RakSleep(300);


	ConsoleServer::DestroyInstance( a );
	LogCommandParser::DestroyInstance( c );
	PacketLogger::DestroyInstance( d );
	RakNetCommandParser::DestroyInstance( e );
	RakNet::RakPeerInterface::DestroyInstance( f );

	return 0;
}

