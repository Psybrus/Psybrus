#pragma once


#include "TestInterface.h"


#include "RakPeerInterface.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include <cstdio>
#include <memory.h>
#include <cstring>
#include <stdlib.h>
#include "Rand.h"
#include "RakNetStatistics.h"
#include "RakSleep.h"
#include "RakMemoryOverride.h"

#include "DebugTools.h"

using namespace RakNet;
class ReliableOrderedConvertedTest : public TestInterface
{
public:
	ReliableOrderedConvertedTest(void);
	~ReliableOrderedConvertedTest(void);
	int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses);//should return 0 if no error, or the error number
	RakString GetTestName();
	RakString ErrorCodeToString(int errorCode);
	void DestroyPeers();
protected:
	void *LoggedMalloc(size_t size, const char *file, unsigned int line);
	void LoggedFree(void *p, const char *file, unsigned int line);
	void* LoggedRealloc(void *p, size_t size, const char *file, unsigned int line);
private:
	DataStructures::List <RakPeerInterface *> destroyList;

};
