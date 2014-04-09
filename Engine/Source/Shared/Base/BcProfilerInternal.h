/**************************************************************************
*
* File:		BcProfilerInternal.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Internal profiler implementation.
*		
*
*
* 
**************************************************************************/

#ifndef __BCPROFILERINTERNAL_H__
#define __BCPROFILERINTERNAL_H__

#include "Base/BcProfiler.h"
#include "Base/BcAtomic.h"
#include "Base/BcMutex.h"
#include "Base/BcString.h"
#include "Base/BcTimer.h"

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// BcProfiler
class BcProfilerInternal:
	public BcProfiler
{
public:
	BcProfilerInternal();
	virtual ~BcProfilerInternal();

	virtual void beginProfiling();
	virtual void endProfiling();
	virtual void registerThreadId( BcThreadId Id, const BcChar* Name );
	virtual void enterSection( const BcChar* Tag );
	virtual void exitSection();

protected:
	struct TProfilerSection
	{
		TProfilerSection():
			Tag_( "" ),
			StartTime_( 0.0f ),
			EndTime_( 0.0f ),
			Parent_( nullptr ),
			Child_( nullptr ),
			Next_( nullptr )
		{
		}

		std::string			Tag_;
		BcF64				StartTime_;
		BcF64				EndTime_;
		
		TProfilerSection*	Parent_;
		TProfilerSection*	Child_;
		TProfilerSection*	Next_;
	};

	struct TProfilerThread
	{
		std::string			Name_;
		BcThreadId			Id_;
	};
	
	TProfilerSection* allocSection();

	std::string dumpSection( TProfilerThread* Thread, TProfilerSection* Section );

private:
	typedef std::map< BcThreadId, TProfilerSection* > TProfilerSectionMap;
	typedef std::vector< TProfilerSection > TProfilerSectionVector;
	typedef std::vector< TProfilerThread > TProfilerThreadVector;
	
	TProfilerSectionVector ProfilerSectionPool_;
	BcAtomic< BcU32 > ProfilerSectionIndex_;
	BcTimer Timer_;

	TProfilerThreadVector Threads_;
	TProfilerSectionMap PerThreadProfilerSections_;

	BcAtomic< BcU32 > BeginCount_;
	BcAtomic< BcU32 > ProfilingActive_;
	BcMutex RegisterLock_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

#endif // PSY_USE_PROFILER



#endif // __BCPROFILERINTERNAL_H__
