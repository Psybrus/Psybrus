#pragma once


#include "RakString.h"
#include "DS_List.h"

using namespace RakNet;
class TestInterface
{
public:
	TestInterface();
	virtual ~TestInterface();
	virtual int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses)=0;//should return 0 if no error, or the error number
	virtual RakString GetTestName()=0;
	virtual RakString ErrorCodeToString(int errorCode)=0;
	virtual void DestroyPeers()=0;
};
