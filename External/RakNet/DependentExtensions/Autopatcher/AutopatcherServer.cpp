/// \file
/// \brief The server plugin for the autopatcher.  Must be running for the client to get patches.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "AutopatcherServer.h"
#include "DirectoryDeltaTransfer.h"
#include "FileList.h"
#include "StringCompressor.h"
#include "RakPeerInterface.h"
#include "FileListTransfer.h"
#include "FileListTransferCBInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "AutopatcherRepositoryInterface.h"
#include "RakAssert.h"
#include "AutopatcherPatchContext.h"
#include <stdio.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning( push )
#endif

using namespace RakNet;

const static unsigned HASH_LENGTH=4;

void AutopatcherServerLoadNotifier_Printf::OnQueueUpdate(SystemAddress remoteSystem, AutopatcherServerLoadNotifier::RequestType requestType, AutopatcherServerLoadNotifier::QueueOperation queueOperation, AutopatcherServerLoadNotifier::AutopatcherState *autopatcherState)
{
	char *operationString;
	char *requestTypeString;
	char systemAddressString[32];
	remoteSystem.ToString(true, systemAddressString);
	if (requestType==ASUMC_GET_CHANGELIST)
		requestTypeString="GetChangelist";
	else
		requestTypeString="GetPatch";
	if (queueOperation==QO_WAS_ADDED)
		operationString="added";
	else if (queueOperation==QO_POPPED_ONTO_TO_PROCESSING_THREAD)
		operationString="processing";
	else if (queueOperation==QO_WAS_ABORTED)
		operationString="aborted";

	printf("%s %s %s. %i queued. %i working.\n", systemAddressString, requestTypeString, operationString, autopatcherState->requestsQueued, autopatcherState->requestsWorking);
}
void AutopatcherServerLoadNotifier_Printf::OnGetChangelistCompleted(
									  SystemAddress remoteSystem,
									  AutopatcherServerLoadNotifier::GetChangelistResult getChangelistResult,
									  AutopatcherServerLoadNotifier::AutopatcherState *autopatcherState)
{
	char systemAddressString[32];
	remoteSystem.ToString(true, systemAddressString);

	char *changelistString;
	if (getChangelistResult==GCR_DELETE_FILES)
		changelistString="Delete files";
	else if (getChangelistResult==GCR_ADD_FILES)
		changelistString="Add files";
	else if (getChangelistResult==GCR_ADD_AND_DELETE_FILES)
		changelistString="Add and delete files";
	else if (getChangelistResult==GCR_NOTHING_TO_DO)
		changelistString="No files in changelist";
	else if (getChangelistResult==GCR_REPOSITORY_ERROR)
		changelistString="Repository error";

	printf("%s GetChangelist complete. %s. %i queued. %i working.\n", systemAddressString, changelistString, autopatcherState->requestsQueued, autopatcherState->requestsWorking);
}
void AutopatcherServerLoadNotifier_Printf::OnGetPatchCompleted(SystemAddress remoteSystem, AutopatcherServerLoadNotifier::PatchResult patchResult, AutopatcherServerLoadNotifier::AutopatcherState *autopatcherState)
{
	char systemAddressString[32];
	remoteSystem.ToString(true, systemAddressString);

	char *patchResultString;
	if (patchResult==PR_NO_FILES_NEEDED_PATCHING)
		patchResultString="No files needed patching"; 
	else if (patchResult==PR_REPOSITORY_ERROR)
		patchResultString="Repository error"; 
	else if (patchResult==PR_PATCHES_WERE_SENT)
		patchResultString="Files pushed for patching";
	else if (patchResult==PR_ABORTED_FROM_INPUT_THREAD)
		patchResultString="Aborted from input thread";
	else if (patchResult==PR_ABORTED_FROM_DOWNLOAD_THREAD)
		patchResultString="Aborted from download thread";

	printf("%s GetPatch complete. %s. %i queued. %i working.\n", systemAddressString, patchResultString, autopatcherState->requestsQueued, autopatcherState->requestsWorking);
}

AutopatcherServer::AutopatcherServer()
{
	fileListTransfer=0;
	priority=HIGH_PRIORITY;
	orderingChannel=0;
//	repository=0;
	maxConcurrentUsers=0;
	loadNotifier=0;
	cache_minTime=0;
	cache_maxTime=0;
	cacheLoaded=false;
}
AutopatcherServer::~AutopatcherServer()
{
	Clear();
}
void AutopatcherServer::SetUploadSendParameters(PacketPriority _priority, char _orderingChannel)
{
	priority=_priority;
	orderingChannel=_orderingChannel;
}
void AutopatcherServer::SetFileListTransferPlugin(FileListTransfer *flt)
{
	if (fileListTransfer)
		fileListTransfer->RemoveCallback(this);
	fileListTransfer=flt;
	if (fileListTransfer)
		fileListTransfer->AddCallback(this);
}
void AutopatcherServer::StartThreads(int numThreads, int numSQLConnections, AutopatcherRepositoryInterface **sqlConnectionPtrArray)
{
	connectionPoolMutex.Lock();
	for (int i=0; i < numSQLConnections; i++)
	{
		// Test the pointers passed, in case the user incorrectly casted an array of a different type
		sqlConnectionPtrArray[i]->GetLastError();
		connectionPool.Push(sqlConnectionPtrArray[i],_FILE_AND_LINE_);
	}
	connectionPoolMutex.Unlock();
	threadPool.SetThreadDataInterface(this,0);
	threadPool.StartThreads(numThreads, 0);
}
void AutopatcherServer::CacheMostRecentPatch(const char *applicationName)
{
	if (connectionPool.Size()>0)
	{
		if (applicationName)
			cache_appName=applicationName;
		else
			cache_appName.Clear();
		cache_patchedFiles.Clear();
		cache_updatedFiles.Clear();
		cache_deletedFiles.Clear();
		cache_updatedFileHashes.Clear();
		cache_minTime=0;
		cache_maxTime=0;

		cacheLoaded = connectionPool[0]->GetMostRecentChangelistWithPatches(cache_appName, &cache_patchedFiles, &cache_updatedFiles, &cache_updatedFileHashes, &cache_deletedFiles, &cache_minTime, &cache_maxTime);
	}
}
void AutopatcherServer::OnAttach(void)
{
}
void AutopatcherServer::OnDetach(void)
{
	Clear();
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherServer::Update(void)
{
	while (PatchingUserLimitReached()==false && userRequestWaitingQueue.Size()>0)
	{
		Packet *packet = PopOffWaitingQueue();
		switch (packet->data[0]) 
		{
		case ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE:
			OnGetChangelistSinceDateInt(packet);
			break;
		case ID_AUTOPATCHER_GET_PATCH:
			OnGetPatchInt(packet);
			break;
		}
		DeallocPacketUnified(packet);
	}
}
PluginReceiveResult AutopatcherServer::OnReceive(Packet *packet)
{
	switch (packet->data[0]) 
	{
	case ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE:
		return OnGetChangelistSinceDate(packet);
	case ID_AUTOPATCHER_GET_PATCH:
		return OnGetPatch(packet);
	}

	return RR_CONTINUE_PROCESSING;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherServer::OnShutdown(void)
{
	Clear();
}
void AutopatcherServer::Clear(void)
{
	// Clear the waiting input and output from the thread pool.
	unsigned i;
	threadPool.StopThreads();
	for (i=0; i < threadPool.InputSize(); i++)
	{
		if (DecrementPatchingUserCount(threadPool.GetInputAtIndex(i).systemAddress))
			CallPatchCompleteCallback(threadPool.GetInputAtIndex(i).systemAddress, AutopatcherServerLoadNotifier::PR_ABORTED_FROM_INPUT_THREAD);
		RakNet::OP_DELETE(threadPool.GetInputAtIndex(i).clientList, _FILE_AND_LINE_);
	}
	threadPool.ClearInput();
	for (i=0; i < threadPool.OutputSize(); i++)
	{
		RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->patchList, _FILE_AND_LINE_);
		RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->deletedFiles, _FILE_AND_LINE_);
		RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->addedFiles, _FILE_AND_LINE_);
	}
	threadPool.ClearOutput();

	while (userRequestWaitingQueue.Size())
		DeallocPacketUnified(AbortOffWaitingQueue());

	patchingUsers.Clear(true, _FILE_AND_LINE_);
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherServer::OnStartup(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherServer::OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	RemoveFromThreadPool(systemAddress);

	unsigned int i=0;
	patchingUsersMutex.Lock();
	while (i < patchingUsers.Size())
	{
		if (patchingUsers[i]==systemAddress)
			patchingUsers.RemoveAtIndexFast(i);
		else
			i++;
	}
	patchingUsersMutex.Unlock();

	while (userRequestWaitingQueue.Size())
		DeallocPacketUnified(AbortOffWaitingQueue());
}
void AutopatcherServer::RemoveFromThreadPool(SystemAddress systemAddress)
{
	unsigned i;
	i=0;
	threadPool.LockInput();
	while (i < threadPool.InputSize())
	{
		if (threadPool.GetInputAtIndex(i).systemAddress==systemAddress)
		{
			if (DecrementPatchingUserCount(systemAddress))
				CallPatchCompleteCallback(threadPool.GetInputAtIndex(i).systemAddress, AutopatcherServerLoadNotifier::PR_ABORTED_FROM_INPUT_THREAD);
			RakNet::OP_DELETE(threadPool.GetInputAtIndex(i).clientList, _FILE_AND_LINE_);
			threadPool.RemoveInputAtIndex(i);
		}
		else
			i++;
	}
	threadPool.UnlockInput();

	i=0;
	threadPool.LockOutput();
	while (i < threadPool.OutputSize())
	{
		if (threadPool.GetOutputAtIndex(i)->systemAddress==systemAddress)
		{
			RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->patchList, _FILE_AND_LINE_);
			RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->deletedFiles, _FILE_AND_LINE_);
			RakNet::OP_DELETE(threadPool.GetOutputAtIndex(i)->addedFiles, _FILE_AND_LINE_);
			threadPool.RemoveOutputAtIndex(i);
		}
		else
			i++;
	}
	threadPool.UnlockOutput();
}
namespace RakNet
{
AutopatcherServer::ResultTypeAndBitstream* GetChangelistSinceDateCB(AutopatcherServer::ThreadData threadData, bool *returnOutput, void* perThreadData)
{
	AutopatcherRepositoryInterface *repository = (AutopatcherRepositoryInterface*)perThreadData;
	
	FileList addedFiles, deletedFiles;
	AutopatcherServer *server = threadData.server;

	//AutopatcherServer::ResultTypeAndBitstream *rtab = RakNet::OP_NEW<AutopatcherServer::ResultTypeAndBitstream>( _FILE_AND_LINE_ );
	AutopatcherServer::ResultTypeAndBitstream rtab;
	rtab.systemAddress=threadData.systemAddress;
// 	rtab.deletedFiles=RakNet::OP_NEW<FileList>( _FILE_AND_LINE_ );
// 	rtab.addedFiles=RakNet::OP_NEW<FileList>( _FILE_AND_LINE_ );
	rtab.deletedFiles=&deletedFiles;
	rtab.addedFiles=&addedFiles;

	// Query the database for a changelist since this date
	RakAssert(server);
	//if (server->repository->GetChangelistSinceDate(threadData.applicationName.C_String(), rtab.addedFiles, rtab.deletedFiles, threadData.lastUpdateDate.C_String(), currentDate))
	if (repository->GetChangelistSinceDate(threadData.applicationName.C_String(), rtab.addedFiles, rtab.deletedFiles, threadData.lastUpdateDate))
	{
		rtab.fatalError=false;
	}
	else
	{
		rtab.fatalError=true;
	}

	rtab.operation=AutopatcherServer::ResultTypeAndBitstream::GET_CHANGELIST_SINCE_DATE;
	rtab.currentDate=(double) time(NULL);
	// *returnOutput=true;
	// return rtab;

	if (rtab.fatalError==false)
	{
		if (rtab.deletedFiles->fileList.Size())
		{
			rtab.bitStream1.Write((unsigned char) ID_AUTOPATCHER_DELETION_LIST);
			rtab.deletedFiles->Serialize(&rtab.bitStream1);
		}

		if (rtab.addedFiles->fileList.Size())
		{
			rtab.bitStream2.Write((unsigned char) ID_AUTOPATCHER_CREATION_LIST);
			rtab.addedFiles->Serialize(&rtab.bitStream2);
			rtab.bitStream2.Write(rtab.currentDate);
			rtab.bitStream2.WriteCasted<double>(0);

			rtab.addedFiles->Clear();
		}
		else
		{
			rtab.bitStream2.Write((unsigned char) ID_AUTOPATCHER_FINISHED);
			rtab.bitStream2.Write(rtab.currentDate);
		}
	}
	else
	{
		rtab.bitStream2.Write((unsigned char) ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR);
		StringCompressor::Instance()->EncodeString(repository->GetLastError(), 256, &rtab.bitStream2);	
	}
// 	RakNet::OP_DELETE(rtab.deletedFiles, _FILE_AND_LINE_);
// 	RakNet::OP_DELETE(rtab.addedFiles, _FILE_AND_LINE_);

	*returnOutput=false;

	if (server->DecrementPatchingUserCount(rtab.systemAddress))
	{
		if (rtab.bitStream1.GetNumberOfBitsUsed()>0)
			server->SendUnified(&(rtab.bitStream1), server->priority, RELIABLE_ORDERED, server->orderingChannel, rtab.systemAddress, false);
		if (rtab.bitStream2.GetNumberOfBitsUsed()>0)
			server->SendUnified(&(rtab.bitStream2), server->priority, RELIABLE_ORDERED, server->orderingChannel, rtab.systemAddress, false);

		if (server->loadNotifier)
		{
			AutopatcherServerLoadNotifier::AutopatcherState autopatcherState;
			autopatcherState.requestsQueued=server->userRequestWaitingQueue.Size();
			autopatcherState.requestsWorking=server->patchingUsers.Size();

			AutopatcherServerLoadNotifier::GetChangelistResult getChangelistResult;
			if (rtab.fatalError==true)
				getChangelistResult=AutopatcherServerLoadNotifier::GCR_REPOSITORY_ERROR;
			else if (rtab.deletedFiles->fileList.Size()==0 && rtab.addedFiles->fileList.Size()==0)
				getChangelistResult=AutopatcherServerLoadNotifier::GCR_NOTHING_TO_DO;
			else if (rtab.deletedFiles->fileList.Size()==0)
				getChangelistResult=AutopatcherServerLoadNotifier::GCR_ADD_FILES;
			else if (rtab.addedFiles->fileList.Size()==0)
				getChangelistResult=AutopatcherServerLoadNotifier::GCR_DELETE_FILES;
			else
				getChangelistResult=AutopatcherServerLoadNotifier::GCR_ADD_AND_DELETE_FILES;

			server->loadNotifier->OnGetChangelistCompleted(rtab.systemAddress, getChangelistResult, &autopatcherState);
		}
	}


	return 0;
}
}
PluginReceiveResult AutopatcherServer::OnGetChangelistSinceDate(Packet *packet)
{
	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	ThreadData threadData;
	inBitStream.IgnoreBits(8);
	inBitStream.ReadCompressed(threadData.applicationName);
	inBitStream.Read(threadData.lastUpdateDate);

	if (cacheLoaded && threadData.lastUpdateDate!=0 && threadData.applicationName==cache_appName)
	{
		RakNet::BitStream bitStream1;
		RakNet::BitStream bitStream2;
		double currentDate=(double) time(NULL);
		if (cache_maxTime!=0 && threadData.lastUpdateDate>cache_maxTime)
		{
			bitStream2.Write((unsigned char) ID_AUTOPATCHER_FINISHED);
			bitStream2.Write(currentDate);
			SendUnified(&bitStream2, priority, RELIABLE_ORDERED,orderingChannel, packet->systemAddress, false);
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}

		// Check in-memory cache, use if possible rather than accessing database
		if (cache_minTime!=0 && threadData.lastUpdateDate>cache_minTime)
		{
			if (cache_deletedFiles.fileList.Size())
			{
				bitStream1.Write((unsigned char) ID_AUTOPATCHER_DELETION_LIST);
				cache_deletedFiles.Serialize(&bitStream1);
				SendUnified(&bitStream1, priority, RELIABLE_ORDERED,orderingChannel, packet->systemAddress, false);
			}
			if (cache_updatedFileHashes.fileList.Size())
			{
				bitStream2.Write((unsigned char) ID_AUTOPATCHER_CREATION_LIST);
				cache_updatedFileHashes.Serialize(&bitStream2);
				bitStream2.Write(currentDate);
				bitStream2.Write(threadData.lastUpdateDate);
			}
			else
			{
				bitStream2.Write((unsigned char) ID_AUTOPATCHER_FINISHED);
				bitStream2.Write(currentDate);
			}
			SendUnified(&bitStream2, priority, RELIABLE_ORDERED,orderingChannel, packet->systemAddress, false);

			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}
	}

	if (PatchingUserLimitReached())
	{
		AddToWaitingQueue(packet);
		return RR_STOP_PROCESSING;
	}

	OnGetChangelistSinceDateInt(packet);
	return RR_STOP_PROCESSING_AND_DEALLOCATE;
}
void AutopatcherServer::OnGetChangelistSinceDateInt(Packet *packet)
{
	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	ThreadData threadData;
	inBitStream.IgnoreBits(8);
	inBitStream.ReadCompressed(threadData.applicationName);
	inBitStream.Read(threadData.lastUpdateDate);

	if (IncrementPatchingUserCount(packet->systemAddress))
	{
		CallPacketCallback(packet, AutopatcherServerLoadNotifier::QO_POPPED_ONTO_TO_PROCESSING_THREAD);

		threadData.server=this;
		threadData.systemAddress=packet->systemAddress;
		threadPool.AddInput(GetChangelistSinceDateCB, threadData);
	}
}
namespace RakNet {
AutopatcherServer::ResultTypeAndBitstream* GetPatchCB(AutopatcherServer::ThreadData threadData, bool *returnOutput, void* perThreadData)
{
	AutopatcherServer *server = threadData.server;
	AutopatcherRepositoryInterface *repository = (AutopatcherRepositoryInterface*)perThreadData;

	// AutopatcherServer::ResultTypeAndBitstream *rtab = RakNet::OP_NEW<AutopatcherServer::ResultTypeAndBitstream>( _FILE_AND_LINE_ );
	AutopatcherServer::ResultTypeAndBitstream rtab;
	rtab.systemAddress=threadData.systemAddress;
	FileList fileList;
	// rtab.patchList=RakNet::OP_NEW<FileList>( _FILE_AND_LINE_ );
	rtab.patchList=&fileList;
	RakAssert(server);
//	RakAssert(server->repository);
//	if (server->repository->GetPatches(threadData.applicationName.C_String(), threadData.clientList, rtab.patchList, currentDate))
	if (repository->GetPatches(threadData.applicationName.C_String(), threadData.clientList, rtab.patchList))
		rtab.fatalError=false;
	else
		rtab.fatalError=true;
	rtab.operation=AutopatcherServer::ResultTypeAndBitstream::GET_PATCH;
	rtab.setId=threadData.setId;
	rtab.currentDate=(double) time(NULL);

	RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);

	if (rtab.fatalError==false)
	{
		if (rtab.patchList->fileList.Size())
		{
			//server->fileListTransfer->Send(rtab.patchList, 0, rtab.systemAddress, rtab.setId, server->priority, server->orderingChannel, false, server->repository);
			server->fileListTransfer->Send(rtab.patchList, 0, rtab.systemAddress, rtab.setId, server->priority, server->orderingChannel, repository, repository->GetIncrementalReadChunkSize());
		}
		else
		{
			// No files needed to send
			if (server->DecrementPatchingUserCount(rtab.systemAddress))
				server->CallPatchCompleteCallback(rtab.systemAddress, AutopatcherServerLoadNotifier::PR_NO_FILES_NEEDED_PATCHING);
		}

		rtab.bitStream1.Write((unsigned char) ID_AUTOPATCHER_FINISHED_INTERNAL);
		rtab.bitStream1.Write(rtab.currentDate);
	}
	else
	{
		rtab.bitStream1.Write((unsigned char) ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR);
	//	StringCompressor::Instance()->EncodeString(server->repository->GetLastError(), 256, &rtab.bitStream1);
		StringCompressor::Instance()->EncodeString(repository->GetLastError(), 256, &rtab.bitStream1);

		if (server->DecrementPatchingUserCount(rtab.systemAddress))
		{
			server->CallPatchCompleteCallback(rtab.systemAddress, AutopatcherServerLoadNotifier::PR_REPOSITORY_ERROR);
		}
		else
		{
			*returnOutput=false;
			return 0;
		}
	}

	*returnOutput=false;

	if (rtab.bitStream1.GetNumberOfBitsUsed()>0)
		server->SendUnified(&(rtab.bitStream1), server->priority, RELIABLE_ORDERED, server->orderingChannel, rtab.systemAddress, false);
	if (rtab.bitStream2.GetNumberOfBitsUsed()>0)
		server->SendUnified(&(rtab.bitStream2), server->priority, RELIABLE_ORDERED, server->orderingChannel, rtab.systemAddress, false);

	// 12/1/2010 This doesn't scale well. Changing to allocating a connection object per request
	/*
	// Wait for repository to finish
	// This is so that the same sql connection is not used between two different plugins, which causes thrashing and bad performance
	// Plus if fileListTransfer uses multiple threads, this will keep this thread and the fileListTransfer thread from using the same connection at the same time
	// PostgreSQL possibly MySQL are not threadsafe for multiple threads on the same connection
	int pendingFiles = server->fileListTransfer->GetPendingFilesToAddress(rtab.systemAddress);
	while (pendingFiles>0)
	{
		RakSleep(pendingFiles*10);
		pendingFiles = server->fileListTransfer->GetPendingFilesToAddress(rtab.systemAddress);
	}
	*/
	
	// *returnOutput=true;
	// return rtab;
	return 0;
}
}
PluginReceiveResult AutopatcherServer::OnGetPatch(Packet *packet)
{
	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	
	ThreadData threadData;
	inBitStream.IgnoreBits(8);
	inBitStream.Read(threadData.setId);
	double lastUpdateDate;
	inBitStream.Read(lastUpdateDate);
	inBitStream.ReadCompressed(threadData.applicationName);
	threadData.clientList=0;

	// Check in-memory cache, use if possible rather than accessing database
	if (threadData.applicationName==cache_appName && lastUpdateDate!=0 && cacheLoaded && cache_minTime!=0 && lastUpdateDate>cache_minTime)
	{
		threadData.systemAddress=packet->systemAddress;
		threadData.server=this;
		threadData.clientList=RakNet::OP_NEW<FileList>( _FILE_AND_LINE_ );

		if (threadData.clientList->Deserialize(&inBitStream)==false)
		{
			RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}
		if (threadData.clientList->fileList.Size()==0)
		{
			RakAssert(0);
			RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}

		char *userHash;
		RakNet::RakString userFilename;
		FileList patchList;

		unsigned int i,j;
		for (i=0; i < threadData.clientList->fileList.Size(); i++)
		{
			userHash=threadData.clientList->fileList[i].data;
			userFilename=threadData.clientList->fileList[i].filename;
			bool sentAnything=false;

			// If the user file has a hash, check this hash against the hash stored with the patch, for the file of the same name
			if (userHash)
			{
				if (threadData.clientList->fileList[i].dataLengthBytes!=HASH_LENGTH)
				{
					RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}

				for (j=0; j < cache_patchedFiles.fileList.Size(); j++)
				{
					if (userFilename == cache_patchedFiles.fileList[j].filename)
					{			
						if (memcmp(cache_patchedFiles.fileList[j].data, userHash, HASH_LENGTH)!=0)
						{
							// Full file will be sent below							
						}
						else
						{
							// Send patch
							RakAssert(cache_patchedFiles.fileList[j].context.op==PC_HASH_2_WITH_PATCH);
							patchList.AddFile(userFilename,userFilename, 0, cache_patchedFiles.fileList[j].dataLengthBytes, cache_patchedFiles.fileList[j].fileLengthBytes, cache_patchedFiles.fileList[j].context, true, false);
							sentAnything=true;
						}

						break;
					}
				}
			}

			if (sentAnything==false)
			{
				RakAssert(userFilename == cache_updatedFiles.fileList[j].filename);

				patchList.AddFile(userFilename,userFilename, 0, cache_updatedFiles.fileList[j].dataLengthBytes, cache_updatedFiles.fileList[j].fileLengthBytes, cache_updatedFiles.fileList[j].context, true, false);
				sentAnything=true;
				break;
			}

			if (sentAnything==false)
			{
				// Failure to find file in cache
				// Will fall to use database
				patchList.Clear();
				break;
			}
		}

		if (patchList.fileList.Size()>0)
		{
			if (IncrementPatchingUserCount(packet->systemAddress))
			{
				fileListTransfer->Send(&patchList, 0, packet->systemAddress, threadData.setId, priority, orderingChannel, this, 262144*4*4);
				RakNet::BitStream bitStream1;
				bitStream1.Write((unsigned char) ID_AUTOPATCHER_FINISHED_INTERNAL);
				double t =(double) time(NULL);
				bitStream1.Write(t);
				SendUnified(&bitStream1, priority, RELIABLE_ORDERED, orderingChannel, packet->systemAddress, false);

				RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			}
		}
	}

	RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
	
	if (PatchingUserLimitReached())
	{
		AddToWaitingQueue(packet);
		return RR_STOP_PROCESSING;
	}

	OnGetPatchInt(packet);
	return RR_STOP_PROCESSING_AND_DEALLOCATE;
}
void AutopatcherServer::OnGetPatchInt(Packet *packet)
{
	RakNet::BitStream inBitStream(packet->data, packet->length, false);

	ThreadData threadData;
	inBitStream.IgnoreBits(8);
	inBitStream.Read(threadData.setId);
	double lastUpdateDate;
	inBitStream.Read(lastUpdateDate);
	inBitStream.ReadCompressed(threadData.applicationName);
	threadData.systemAddress=packet->systemAddress;
	threadData.server=this;
	threadData.clientList=RakNet::OP_NEW<FileList>( _FILE_AND_LINE_ );

	if (threadData.clientList->Deserialize(&inBitStream)==false)
	{
		RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
		return;
	}
	if (threadData.clientList->fileList.Size()==0)
	{
		RakAssert(0);
		RakNet::OP_DELETE(threadData.clientList, _FILE_AND_LINE_);
		return;
	}

	if (IncrementPatchingUserCount(packet->systemAddress))
		CallPacketCallback(packet, AutopatcherServerLoadNotifier::QO_POPPED_ONTO_TO_PROCESSING_THREAD);

	threadPool.AddInput(GetPatchCB, threadData);
}
void* AutopatcherServer::PerThreadFactory(void *context)
{
	(void)context;

	AutopatcherRepositoryInterface* p;
	connectionPoolMutex.Lock();
	p=connectionPool.Pop();
	connectionPoolMutex.Unlock();
	return p;
}
void AutopatcherServer::PerThreadDestructor(void* factoryResult, void *context)
{
	(void)context;
	(void)factoryResult;
}
void AutopatcherServer::OnFilePushesComplete( SystemAddress systemAddress, unsigned short setID )
{
	if (DecrementPatchingUserCount(systemAddress))
		CallPatchCompleteCallback(systemAddress, AutopatcherServerLoadNotifier::PR_PATCHES_WERE_SENT);
}
void AutopatcherServer::OnSendAborted( SystemAddress systemAddress )
{
	if (DecrementPatchingUserCount(systemAddress))
		CallPatchCompleteCallback(systemAddress, AutopatcherServerLoadNotifier::PR_ABORTED_FROM_DOWNLOAD_THREAD);
}
bool AutopatcherServer::IncrementPatchingUserCount(SystemAddress sa)
{
	// A system address may exist more than once in patchingUsers
	patchingUsersMutex.Lock();
	patchingUsers.Insert(sa, _FILE_AND_LINE_);
	patchingUsersMutex.Unlock();
	return true;
}
bool AutopatcherServer::DecrementPatchingUserCount(SystemAddress sa)
{
	unsigned int i;
	patchingUsersMutex.Lock();
	for (i=0; i < patchingUsers.Size(); i++)
	{
		if (patchingUsers[i]==sa)
		{
			patchingUsers.RemoveAtIndexFast(i);
			patchingUsersMutex.Unlock();
			return true;
		}
	}
	patchingUsersMutex.Unlock();
	return false;
}
bool AutopatcherServer::PatchingUserLimitReached(void) const
{
	if (maxConcurrentUsers==0)
		return false;

	return patchingUsers.Size()>=maxConcurrentUsers;
}
void AutopatcherServer::SetMaxConurrentUsers(unsigned int _maxConcurrentUsers)
{
	maxConcurrentUsers=_maxConcurrentUsers;
}
void AutopatcherServer::CallPacketCallback(Packet *packet, AutopatcherServerLoadNotifier::QueueOperation queueOperation)
{
	if (loadNotifier)
	{
		AutopatcherServerLoadNotifier::AutopatcherState autopatcherState;
		autopatcherState.requestsQueued=userRequestWaitingQueue.Size();
		autopatcherState.requestsWorking=patchingUsers.Size();

		AutopatcherServerLoadNotifier::RequestType requestType;
		if (packet->data[0]==ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE)
			requestType=AutopatcherServerLoadNotifier::ASUMC_GET_CHANGELIST;
		else
			requestType=AutopatcherServerLoadNotifier::ASUMC_GET_PATCH;

		loadNotifier->OnQueueUpdate(packet->systemAddress, requestType, queueOperation, &autopatcherState);
	}
}
void AutopatcherServer::CallPatchCompleteCallback(const SystemAddress &systemAddress, AutopatcherServerLoadNotifier::PatchResult patchResult)
{
	if (loadNotifier)
	{
		AutopatcherServerLoadNotifier::AutopatcherState autopatcherState;
		autopatcherState.requestsQueued=userRequestWaitingQueue.Size();
		autopatcherState.requestsWorking=patchingUsers.Size();

		loadNotifier->OnGetPatchCompleted(systemAddress, patchResult, &autopatcherState);
	}
}
void AutopatcherServer::AddToWaitingQueue(Packet *packet)
{
	userRequestWaitingQueue.Push(packet, _FILE_AND_LINE_);
	CallPacketCallback(packet, AutopatcherServerLoadNotifier::QO_WAS_ADDED);
}
Packet *AutopatcherServer::AbortOffWaitingQueue(void)
{
	Packet *packet = userRequestWaitingQueue.Pop();
	CallPacketCallback(packet,AutopatcherServerLoadNotifier::QO_WAS_ABORTED);
	return packet;
}
Packet *AutopatcherServer::PopOffWaitingQueue(void)
{
	return userRequestWaitingQueue.Pop();;
}
void AutopatcherServer::SetLoadManagementCallback(AutopatcherServerLoadNotifier *asumc)
{
	loadNotifier=asumc;
}
unsigned int AutopatcherServer::GetFilePart( const char *filename, unsigned int startReadBytes, unsigned int numBytesToRead, void *preallocatedDestination, FileListNodeContext context)
{
	/*
	int offset;
	if (context.op==PC_HASH_1_WITH_PATCH)
		offset=HASH_LENGTH;
	else if (context.op==PC_HASH_2_WITH_PATCH)
		offset=HASH_LENGTH*2;
	else
		offset=0;

	int bytesToRead;
	if (startReadBytes + numBytesToRead > context.dataLength-offset)
		bytesToRead=(context.dataLength-offset)-startReadBytes;
	else
		bytesToRead=numBytesToRead;

	memcpy(preallocatedDestination, ((char*)context.dataPtr)+offset, bytesToRead);
	*/

	int bytesToRead;
	if (startReadBytes + numBytesToRead > context.dataLength)
		bytesToRead=(context.dataLength)-startReadBytes;
	else
		bytesToRead=numBytesToRead;

	memcpy(preallocatedDestination, context.dataPtr, bytesToRead);
	return bytesToRead;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
