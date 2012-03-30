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
#include "CommonFunctions.h"

#include <stdlib.h> // For atoi
#include <cstring> // For strlen
#include "Rand.h"
#include <stdio.h>

using namespace RakNet;
class ComprehensiveConvertTest : public TestInterface
{
public:
	ComprehensiveConvertTest(void);
	~ComprehensiveConvertTest(void);
	int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses);//should return 0 if no error, or the error number
	RakString GetTestName();
	RakString ErrorCodeToString(int errorCode);
	void DestroyPeers();
private:
	static const int NUM_PEERS =10;
	RakPeerInterface *peers[NUM_PEERS];

};
