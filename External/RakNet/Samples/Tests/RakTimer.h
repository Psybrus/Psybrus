#pragma once


#include "RakNetTime.h"
#include "GetTime.h"

using namespace RakNet;
class RakTimer
{
public:
	RakTimer(void);
	RakTimer(int lengthInMilliseconds);
	~RakTimer(void);
    void SetTimerLength(int lengthInMilliseconds);
    void Start();
    void Pause();
    void Resume();
	bool IsExpired();
private:
	int timerLength;//Modified by SetTimerLength
	int pauseOffset;
	TimeMS startTime;
};
