#ifndef __OSINPUTMINDWAVELINUX_H__
#define __OSINPUTMINDWAVELINUX_H__

#include "System/Os/OsEvents.h"

#include <thread>
#include <deque>
#include <memory>

//////////////////////////////////////////////////////////////////////////
// ThinkGear forward decls.
extern "C"
{
	struct _ThinkGearStreamParser;
}

//////////////////////////////////////////////////////////////////////////
// OsInputMindwaveLinux
class OsInputMindwaveLinux
{
public:
	OsInputMindwaveLinux();
	virtual ~OsInputMindwaveLinux();

	/**
	 * Update mindwave input device.
	 */
	virtual void update();


private:
	void workerThread();

	static void handleDataValue(
		unsigned char ExtendedCodeLevel,
		unsigned char Code, unsigned char NumBytes,
		const unsigned char* Value, void* CustomData );


private:
	enum class State : int
	{
		INIT = 0,
		SCAN,
		CONNECT,
		READ,
		DISCONNECT,
		DEINIT
	};

	std::atomic< State > State_;
	std::atomic< int > Shutdown_;

	int DeviceId_;
	int Socket_;

	OsEventInputMindwave Event_;

	std::unique_ptr< _ThinkGearStreamParser > StreamParser_;
	std::thread WorkerThread_;
};

#endif // __OSINPUTMINDWAVELINUX_H__
