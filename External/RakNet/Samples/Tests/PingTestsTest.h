#pragma once


#include "TestInterface.h"

#include "RakString.h"

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakPeer.h"
#include "RakSleep.h"
#include "RakNetTime.h"
#include "GetTime.h"
#include "DebugTools.h"
#include "TestHelpers.h"
#include "CommonFunctions.h"
#include "RakTimer.h"

using namespace RakNet;
class PingTestsTest : public TestInterface
{
public:
	PingTestsTest(void);
	~PingTestsTest(void);
	int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses);//should return 0 if no error, or the error number
	RakString GetTestName();
	RakString ErrorCodeToString(int errorCode);
	void DestroyPeers();
	int TestAverageValue(int averagePing,int line,bool noPauses,bool isVerbose);
private:
	DataStructures::List <RakPeerInterface *> destroyList;

};
