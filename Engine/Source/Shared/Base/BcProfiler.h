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
	 * Enter section to profile.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void enterSection( const BcChar* Tag ) = 0;

	/**
	 * Exit section.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void exitSection( const BcChar* Tag ) = 0;

	/**
	 * Start async section. Tag MUST be unique.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void startAsync( const BcChar* Tag ) = 0;

	/**
	 * End async section. Tag MUST be unique.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void endAsync( const BcChar* Tag ) = 0;

private:

};

//////////////////////////////////////////////////////////////////////////
// BcProfilerSectionScope
class BcProfilerSectionScope
{
public:
	BcProfilerSectionScope( const std::string& Tag )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			Tag_ = Tag;
			BcProfiler::pImpl()->enterSection( Tag_.c_str() );
		}
	}

	~BcProfilerSectionScope()
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcProfiler::pImpl()->exitSection( Tag_.c_str() );
		}
	}

private:
	std::string Tag_;
};


#define PSY_PROFILER_SECTION( _LocalName, _Tag ) \
	BcProfilerSectionScope _LocalName( _Tag ) 

#else

#define PSY_PROFILER_SECTION( _LocalName, _Tag )

#endif // PSY_USE_PROFILER

#endif // __BCPROFILER_H__