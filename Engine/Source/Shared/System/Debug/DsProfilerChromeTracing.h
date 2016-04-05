/**************************************************************************
*
* File:		DsProfilerChromeTracing.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Chrome Tracing (chrome://tracing) profiler.
*			
*
*
* 
**************************************************************************/

#ifndef __DsProfilerChromeTracing_H__
#define __DsProfilerChromeTracing_H__

#include "Base/BcLinearAllocator.h"
#include "Base/BcProfiler.h"
#include "Base/BcString.h"
#include "Base/BcTimer.h"
#include "System/SysFence.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// BcProfiler
class DsProfilerChromeTracing:
	public BcProfiler
{
public:
	DsProfilerChromeTracing();
	virtual ~DsProfilerChromeTracing();

	void setThreadName( BcThreadId ThreadId, const char* Name ) override;
	void beginProfiling() override;
	void endProfiling() override;
	void enterSection( const char* Tag ) override;
	void exitSection( const char* Tag ) override;
	void startAsync( const char* Tag, void* Data ) override;
	void stepAsync( const char* Tag, void* Data ) override;
	void endAsync( const char* Tag, void* Data ) override;
	void instantEvent( const char* Tag ) override;

protected:
	struct TProfilerEvent
	{
		TProfilerEvent():
			Tag_( reinterpret_cast< char* > ( this + 1 ) ),
			Type_( 0 ),
			Data_( nullptr ),
			ThreadId_( 0 ),
			StartTime_( 0.0f )
		{
		}

		char* Tag_;
		char Type_;
		void* Data_;
		BcThreadId ThreadId_;
		BcF64 StartTime_;
	};

	struct TProfilerThread
	{
		std::string			Name_;
		BcThreadId			Id_;
	};
	
	TProfilerEvent* allocEvent( const char* Tag );

private:
	typedef std::vector< TProfilerEvent* > TProfilerEventVector;
	
	std::mutex InternalMutex_;
	std::unordered_map< BcThreadId, std::string > ThreadNames_;

	TProfilerEventVector ProfilerSections_;
	std::atomic< size_t > ProfilerSectionIdx_;
	BcTimer Timer_;

	std::atomic< BcU32 > BeginCount_;
	std::atomic< BcU32 > ProfilingActive_;

	BcLinearAllocator Allocator_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

#endif // PSY_USE_PROFILER



#endif // __DsProfilerChromeTracing_H__
