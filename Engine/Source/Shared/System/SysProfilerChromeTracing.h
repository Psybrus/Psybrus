/**************************************************************************
*
* File:		SysProfilerChromeTracing.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Chrome Tracing (chrome://tracing) profiler.
*			
*
*
* 
**************************************************************************/

#ifndef __SysProfilerChromeTracing_H__
#define __SysProfilerChromeTracing_H__

#include "Base/BcProfiler.h"
#include "Base/BcAtomic.h"
#include "Base/BcMutex.h"
#include "Base/BcString.h"
#include "Base/BcTimer.h"

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// BcProfiler
class SysProfilerChromeTracing:
	public BcProfiler
{
public:
	SysProfilerChromeTracing();
	virtual ~SysProfilerChromeTracing();

	virtual void beginProfiling();
	virtual void endProfiling();
	virtual void enterSection( const BcChar* Tag );
	virtual void exitSection( const BcChar* Tag );
	virtual void startAsync( const BcChar* Tag );
	virtual void endAsync( const BcChar* Tag );
	virtual void instantEvent( const BcChar* Tag );

protected:
	struct TProfilerEvent
	{
		TProfilerEvent():
			Tag_( "" ),
			Type_( "" ),
			ThreadId_( 0 ),
			StartTime_( 0.0f )
		{
		}

		std::string			Tag_;
		std::string			Type_;
		BcThreadId			ThreadId_;
		BcF64				StartTime_;
	};

	struct TProfilerThread
	{
		std::string			Name_;
		BcThreadId			Id_;
	};
	
	TProfilerEvent* allocEvent();

private:
	typedef std::vector< TProfilerEvent > TProfilerEventVector;
	
	TProfilerEventVector ProfilerSectionPool_;
	BcAtomic< BcU32 > ProfilerSectionIndex_;
	BcTimer Timer_;

	BcAtomic< BcU32 > BeginCount_;
	BcAtomic< BcU32 > ProfilingActive_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

#endif // PSY_USE_PROFILER



#endif // __SysProfilerChromeTracing_H__
