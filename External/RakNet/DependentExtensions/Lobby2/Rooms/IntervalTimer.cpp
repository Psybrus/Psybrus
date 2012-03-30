#include "IntervalTimer.h"

void IntervalTimer::SetPeriod(RakNet::TimeMS period) {basePeriod=period; remaining=0;}
bool IntervalTimer::UpdateInterval(RakNet::TimeMS elapsed)
{
	if (elapsed >= remaining)
	{
		RakNet::TimeMS difference = elapsed-remaining;
		if (difference >= basePeriod)
		{
			remaining=basePeriod;
		}
		else
		{
			remaining=basePeriod-difference;
		}

		return true;
	}

	remaining-=elapsed;
	return false;
}