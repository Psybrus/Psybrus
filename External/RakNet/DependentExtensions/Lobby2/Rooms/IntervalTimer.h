#ifndef __INTERVAL_TIMER_H
#define __INTERVAL_TIMER_H

#include "RakNetTypes.h"

struct IntervalTimer
{
	void SetPeriod(RakNet::TimeMS period);
	bool UpdateInterval(RakNet::TimeMS elapsed);

	RakNet::TimeMS basePeriod, remaining;	
};

#endif