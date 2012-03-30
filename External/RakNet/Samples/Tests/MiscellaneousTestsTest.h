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
#include "TestHelpers.h"


using namespace RakNet;
class MiscellaneousTestsTest : public TestInterface
{
public:
	MiscellaneousTestsTest(void);
	~MiscellaneousTestsTest(void);
	int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses);//should return 0 if no error, or the error number
	RakString GetTestName();
	RakString ErrorCodeToString(int errorCode);
	void DestroyPeers();

		
private:
	DataStructures::List <RakString> errorList;
	DataStructures::List <RakNet::RakPeerInterface *> destroyList;

};
