// Common includes
#include <stdio.h>
#include <stdlib.h>
#include "Kbhit.h"

#include "GetTime.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "StringCompressor.h"
#include "PacketizedTCP.h"

// Client only includes
#include "FileListTransferCBInterface.h"
#include "FileListTransfer.h"
#include "AutopatcherClient.h"
#include "AutopatcherPatchContext.h"
#include "Gets.h"
#include "RakSleep.h"

class TestCB : public RakNet::FileListTransferCBInterface
{
public:
	virtual bool OnFile(OnFileStruct *onFileStruct)
	{
		if (onFileStruct->context.op==PC_HASH_1_WITH_PATCH || onFileStruct->context.op==PC_HASH_2_WITH_PATCH)
			printf("Patched: ");
		else if (onFileStruct->context.op==PC_WRITE_FILE)
			printf("Written: ");
		else if (onFileStruct->context.op==PC_ERROR_FILE_WRITE_FAILURE)
			printf("Write Failure: ");
		else if (onFileStruct->context.op==PC_ERROR_PATCH_TARGET_MISSING)
			printf("Patch target missing: ");
		else if (onFileStruct->context.op==PC_ERROR_PATCH_APPLICATION_FAILURE)
			printf("Patch process failure: ");
		else if (onFileStruct->context.op==PC_ERROR_PATCH_RESULT_CHECKSUM_FAILURE)
			printf("Patch checksum failure: ");
		else if (onFileStruct->context.op==PC_NOTICE_WILL_COPY_ON_RESTART)
			printf("Copy pending restart: ");
		else if (onFileStruct->context.op==PC_NOTICE_FILE_DOWNLOADED)
			printf("Downloaded: ");
		else if (onFileStruct->context.op==PC_NOTICE_FILE_DOWNLOADED_PATCH)
			printf("Downloaded Patch: ");
		else
			RakAssert(0);


		printf("%i. (100%%) %i/%i %s %ib / %ib\n", onFileStruct->setID, onFileStruct->fileIndex+1, onFileStruct->numberOfFilesInThisSet,
			onFileStruct->fileName, onFileStruct->byteLengthOfThisFile,
			onFileStruct->byteLengthOfThisSet);

		// Return false for the file data to be deallocated automatically
		return false;
	}

	virtual void OnFileProgress(FileProgressStruct *fps)
	{
		printf("Downloading: %i. (%i%%) %i/%i %s %ib / %ib\n", fps->onFileStruct->setID, (int) (100.0*(double)fps->partCount/(double)fps->partTotal),
			fps->onFileStruct->fileIndex+1, fps->onFileStruct->numberOfFilesInThisSet, fps->onFileStruct->fileName,
			fps->onFileStruct->byteLengthOfThisFile,
			fps->onFileStruct->byteLengthOfThisSet);
	}

} transferCallback;

#define USE_TCP

int main(int argc, char **argv)
{
	printf("A simple client interface for the advanced autopatcher.\n");
	printf("Use DirectoryDeltaTransfer for a simpler version of an autopatcher.\n");
	printf("Difficulty: Intermediate\n\n");

	printf("Client starting...");
	RakNet::SystemAddress serverAddress=RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	RakNet::AutopatcherClient autopatcherClient;
	RakNet::FileListTransfer fileListTransfer;
	autopatcherClient.SetFileListTransferPlugin(&fileListTransfer);
	unsigned short localPort=0;
	if (argc>=6)
	{
		localPort=atoi(argv[5]);
	}
#ifdef USE_TCP
	RakNet::PacketizedTCP packetizedTCP;
	if (packetizedTCP.Start(localPort,1)==false)
	{
		printf("Failed to start TCP. Is the port already in use?");
		return 1;
	}
	packetizedTCP.AttachPlugin(&autopatcherClient);
	packetizedTCP.AttachPlugin(&fileListTransfer);
#else
	RakNet::RakPeerInterface *rakPeer;
	rakPeer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor socketDescriptor(localPort,0);
	rakPeer->Startup(1,&socketDescriptor, 1);
	// Plugin will send us downloading progress notifications if a file is split to fit under the MTU 10 or more times
	rakPeer->SetSplitMessageProgressInterval(10);
	rakPeer->AttachPlugin(&autopatcherClient);
	rakPeer->AttachPlugin(&fileListTransfer);
#endif
	printf("started\n");
	char buff[512];
	if (argc<2)
	{
		printf("Enter server IP: ");
		Gets(buff,sizeof(buff));
		if (buff[0]==0)
			//strcpy(buff, "94.198.81.195");
			strcpy(buff, "127.0.0.1");
	}
	else
		strcpy(buff, argv[1]);

#ifdef USE_TCP
	packetizedTCP.Connect(buff,60000,false);
#else
	rakPeer->Connect(buff, 60000, 0, 0);
#endif

	printf("Connecting...\n");
	char appDir[512];
	if (argc<3)
	{
		printf("Enter application directory: ");
		Gets(appDir,sizeof(appDir));
		if (appDir[0]==0)
		{
			strcpy(appDir, "D:/temp2");
		}
	}
	else
		strcpy(appDir, argv[2]);
	char appName[512];
	if (argc<4)
	{
		printf("Enter application name: ");
		Gets(appName,sizeof(appName));
		if (appName[0]==0)
			strcpy(appName, "TestApp");
	}
	else
		strcpy(appName, argv[3]);

	bool patchImmediately=argc>=5 && argv[4][0]=='1';

	if (patchImmediately==false)
		printf("Hit 'q' to quit, 'p' to patch, 'c' to cancel the patch. 'r' to reconnect. 'd' to disconnect.\n");
	else
		printf("Hit 'q' to quit, 'c' to cancel the patch.\n");

	char ch;
	RakNet::Packet *p;
	while (1)
	{
#ifdef USE_TCP
		RakNet::SystemAddress notificationAddress;
		notificationAddress=packetizedTCP.HasCompletedConnectionAttempt();
		if (notificationAddress!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
		{
			printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
			serverAddress=notificationAddress;
		}
		notificationAddress=packetizedTCP.HasNewIncomingConnection();
		if (notificationAddress!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			printf("ID_NEW_INCOMING_CONNECTION\n");
		notificationAddress=packetizedTCP.HasLostConnection();
		if (notificationAddress!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			printf("ID_CONNECTION_LOST\n");


		p=packetizedTCP.Receive();
		while (p)
		{
			if (p->data[0]==ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR)
			{
				char buff[256];
				RakNet::BitStream temp(p->data, p->length, false);
				temp.IgnoreBits(8);
				RakNet::StringCompressor::Instance()->DecodeString(buff, 256, &temp);
				printf("ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR\n");
				printf("%s\n", buff);
			}
			else if (p->data[0]==ID_AUTOPATCHER_FINISHED)
			{
				printf("ID_AUTOPATCHER_FINISHED with server time %f\n", autopatcherClient.GetServerDate());
				double srvDate=autopatcherClient.GetServerDate();
				FILE *fp = fopen("srvDate", "wb");
				fwrite(&srvDate,sizeof(double),1,fp);
				fclose(fp);
			}
			else if (p->data[0]==ID_AUTOPATCHER_RESTART_APPLICATION)
				printf("Launch \"AutopatcherClientRestarter.exe autopatcherRestart.txt\"\nQuit this application immediately after to unlock files.\n");

			packetizedTCP.DeallocatePacket(p);
			p=packetizedTCP.Receive();
		}
#else
		p=rakPeer->Receive();
		while (p)
		{
			if (p->data[0]==ID_DISCONNECTION_NOTIFICATION)
				printf("ID_DISCONNECTION_NOTIFICATION\n");
			else if (p->data[0]==ID_CONNECTION_LOST)
				printf("ID_CONNECTION_LOST\n");
			else if (p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
			{
				printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
				serverAddress=p->systemAddress;
			}
			else if (p->data[0]==ID_CONNECTION_ATTEMPT_FAILED)
				printf("ID_CONNECTION_ATTEMPT_FAILED\n");
			else if (p->data[0]==ID_NO_FREE_INCOMING_CONNECTIONS)
				printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
			else if (p->data[0]==ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR)
			{
				char buff[256];
				RakNet::BitStream temp(p->data, p->length, false);
				temp.IgnoreBits(8);
				RakNet::StringCompressor::Instance()->DecodeString(buff, 256, &temp);
				printf("ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR\n");
				printf("%s\n", buff);
			}
			else if (p->data[0]==ID_AUTOPATCHER_FINISHED)
			{
				printf("ID_AUTOPATCHER_FINISHED with server time %f\n", autopatcherClient.GetServerDate());
				double srvDate=autopatcherClient.GetServerDate();
				FILE *fp = fopen("srvDate", "wb");
				fwrite(&srvDate,sizeof(double),1,fp);
				fclose(fp);
			}
			else if (p->data[0]==ID_AUTOPATCHER_RESTART_APPLICATION)
				printf("Launch \"AutopatcherClientRestarter.exe autopatcherRestart.txt\"\nQuit this application immediately after to unlock files.\n");

			rakPeer->DeallocatePacket(p);
			p=rakPeer->Receive();
		}
#endif

		if (kbhit())
			ch=getch();
		else
			ch=0;

		if (ch=='q')
			break;
		else if (ch=='r')
		{
#ifdef USE_TCP
			packetizedTCP.Connect(buff,60000,false);
#else
			rakPeer->Connect(buff, 60000, 0, 0);
#endif
		}
		else if (ch=='d')
		{
#ifdef USE_TCP
			packetizedTCP.CloseConnection(serverAddress);
#else
			rakPeer->CloseConnection(serverAddress, true);
#endif
		}
		else if (ch=='p' || (serverAddress!=RakNet::UNASSIGNED_SYSTEM_ADDRESS && patchImmediately==true))
		{
			patchImmediately=false;
			char restartFile[512];
			strcpy(restartFile, appDir);
			strcat(restartFile, "/autopatcherRestart.txt");

			double lastUpdateDate;
			FILE *fp = fopen("srvDate", "rb");
			if (fp)
			{
				fread(&lastUpdateDate, sizeof(lastUpdateDate), 1, fp);
				fclose(fp);
			}
			else
				lastUpdateDate=0;

			if (autopatcherClient.PatchApplication(appName, appDir, lastUpdateDate, serverAddress, &transferCallback, restartFile, argv[0]))
			{
				printf("Patching process starting.\n");
			}
			else
			{
				printf("Failed to start patching.\n");
			}
		}
		else if (ch=='c')
		{
			autopatcherClient.Clear();
			printf("Autopatcher cleared.\n");
		}

		RakSleep(30);
	}

	// Dereference so the destructor doesn't crash
	autopatcherClient.SetFileListTransferPlugin(0);

#ifdef USE_TCP
	packetizedTCP.Stop();
#else
	rakPeer->Shutdown(500,0);
	RakNet::RakPeerInterface::DestroyInstance(rakPeer);
#endif
	return 1;
}
