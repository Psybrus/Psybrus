/**************************************************************************
*
* File:		BcProfiler.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#ifndef __BCPROFILER_H__
#define __BCPROFILER_H__

#include "Base/BcTypes.h"
#include "Base/BcMisc.h"
#include "Base/BcGlobal.h"

#define PSY_USE_PROFILER 1
#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// BcProfiler
class BcProfiler:
	public BcGlobal< BcProfiler >
{
public:
	BcProfiler(){};
	virtual ~BcProfiler(){};

	/**
	 * Begin profiling.
	 */
	virtual void beginProfiling() = 0;

	/**
	 * End profiling.
	 */
	virtual void endProfiling() = 0;

	/**
	 * Register thread ID to be profiled.
	 * This ensures we won't need to create a profiling
	 * stack for a thread at profile time if need be.
	 */
	virtual void registerThreadId( BcThreadId Id, const BcChar* Name ) = 0;

	/**
	 * Enter section to profile.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void enterSection( const BcChar* Tag ) = 0;

	/**
	 * Exit section.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void exitSection() = 0;

private:

};

//////////////////////////////////////////////////////////////////////////
// BcProfilerSectionScope
class BcProfilerSectionScope
{
public:
	BcProfilerSectionScope( const std::string& Tag )
	{
		BcProfiler::pImpl()->enterSection( Tag.c_str() );
	}

	~BcProfilerSectionScope()
	{
		BcProfiler::pImpl()->exitSection();
	}
};

#define PSY_PROFILER_SECTION( _LocalName, _Tag ) \
	BcProfilerSectionScope _LocalName( _Tag ) 

#else

#define PSY_PROFILER_SECTION( _LocalName, _Tag )

#endif // PSY_USE_PROFILER

#endif // __BCPROFILER_H__