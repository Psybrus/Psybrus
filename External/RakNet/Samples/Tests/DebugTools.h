#pragma once


#include "RakString.h"

using namespace RakNet;
class DebugTools
{
public:
	DebugTools(void);
	~DebugTools(void);
	static void ShowError(RakString errorString,bool pause, unsigned int lineNum,const char *fileName);
};
