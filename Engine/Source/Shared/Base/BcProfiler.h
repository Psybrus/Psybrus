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

#include "Base/BcGlobal.h"
#include "Base/BcMisc.h"
#include "Base/BcString.h"

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
	 * Initialise graphics API profiling.
	 * @param API API. I.e. D3D12, GL, VK, etc.
	 * @param Context Context for API.
	 * @param Device Device to profile.
	 */
	virtual void initialiseGraphics( const char* API, void* Context, void* Device ) = 0;

	/**
	 * Shutdown graphics API profiling.
	 */
	virtual void shutdownGraphics() = 0;

	/**
	 * Set current thread name.
	 */
	virtual void setCurrentThreadName( const char* Name ) = 0;

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
	virtual void enterSection( const char* Tag ) = 0;

	/**
	 * Exit section.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void exitSection( const char* Tag ) = 0;

	/**
	 * Enter GPU section to profile.
	 */
	virtual void enterGPUSection( const char* Tag ) = 0;

	/**
	 * Exit GPU section.
	 */
	virtual void exitGPUSection( const char* Tag ) = 0;

	/**
	 * Start async section. Tag MUST be unique.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void startAsync( const char* Tag, void* Data ) = 0;

	/**
	 * Step async section. Tag MUST be unique.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void stepAsync( const char* Tag, void* Data ) = 0;

	/**
	 * End async section. Tag MUST be unique.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void endAsync( const char* Tag, void* Data ) = 0;

	/**
	 * Instant event.
	 * Should be thread safe, and track appropriately.
	 */
	virtual void instantEvent( const char* Tag ) = 0;

private:

};

//////////////////////////////////////////////////////////////////////////
// BcProfilerSectionScope
class BcProfilerSectionScope
{
public:
	BcProfilerSectionScope( const char* Tag ):
		Tag_( { 0 } )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				BcStrCopy( Tag_.data(), BcU32( Tag_.size() - 1 ), Tag );
				BcProfiler::pImpl()->enterSection( Tag_.data() );
			}
		}
	}

	BcProfilerSectionScope( const std::string& Tag ):
		Tag_( { 0 } )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcStrCopy( Tag_.data(), BcU32( Tag_.size() - 1 ), Tag.c_str() );
			BcProfiler::pImpl()->enterSection( Tag_.data() );
		}
	}

	~BcProfilerSectionScope()
	{
		if( BcProfiler::pImpl() != nullptr && Tag_[ 0 ] != 0 )
		{
			BcProfiler::pImpl()->exitSection( Tag_.data() );
		}
	}

private:
	std::array< char, 128 > Tag_;
};

//////////////////////////////////////////////////////////////////////////
// BcProfilerGPUSectionScope
class BcProfilerGPUSectionScope
{
public:
	BcProfilerGPUSectionScope( const char* Tag ):
		Tag_( { 0 } )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				BcStrCopy( Tag_.data(), BcU32( Tag_.size() - 1 ), Tag );
				BcProfiler::pImpl()->enterGPUSection( Tag_.data() );
			}
		}
	}

	BcProfilerGPUSectionScope( const std::string& Tag ):
		Tag_( { 0 } )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcStrCopy( Tag_.data(), BcU32( Tag_.size() - 1 ), Tag.c_str() );
			BcProfiler::pImpl()->enterGPUSection( Tag_.data() );
		}
	}

	~BcProfilerGPUSectionScope()
	{
		if( BcProfiler::pImpl() != nullptr && Tag_[ 0 ] != 0 )
		{
			BcProfiler::pImpl()->exitGPUSection( Tag_.data() );
		}
	}

private:
	std::array< char, 128 > Tag_;
};

//////////////////////////////////////////////////////////////////////////
// BcProfilerStartAsync
class BcProfilerStartAsync
{
public:
	BcProfilerStartAsync( const char* Tag, void* Data )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				BcProfiler::pImpl()->startAsync( Tag, Data );
			}
		}
	}

	BcProfilerStartAsync( const std::string& Tag, void* Data )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcProfiler::pImpl()->startAsync( Tag.c_str(), Data );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// BcProfilerStepAsync
class BcProfilerStepAsync
{
public:
	BcProfilerStepAsync( const char* Tag, void* Data )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				BcProfiler::pImpl()->stepAsync( Tag, Data );
			}
		}
	}

	BcProfilerStepAsync( const std::string& Tag, void* Data )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcProfiler::pImpl()->stepAsync( Tag.c_str(), Data );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// BcProfilerEndAsync
class BcProfilerEndAsync
{
public:
	BcProfilerEndAsync( const char* Tag, void* Data )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				BcProfiler::pImpl()->endAsync( Tag, Data );
			}
		}
	}

	BcProfilerEndAsync( const std::string& Tag, void* Data )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			BcProfiler::pImpl()->endAsync( Tag.c_str(), Data );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// BcProfilerInstantEvent
class BcProfilerInstantEvent
{
public:
	BcProfilerInstantEvent( const char* Tag, void* Data )
	{
		if( Tag )
		{
			if( BcProfiler::pImpl() != nullptr )
			{
				std::array< char, 128 > Buffer = { 0 };
				BcSPrintf( Buffer.data(), Buffer.size() - 1, "%s[%p]", Tag, Data );
				BcProfiler::pImpl()->instantEvent( Buffer.data() );
			}
		}
	}

	BcProfilerInstantEvent( const std::string& Tag, void* Data )
	{
		if( BcProfiler::pImpl() != nullptr )
		{
			std::array< char, 128 > Buffer = { 0 };
			BcSPrintf( Buffer.data(), Buffer.size() - 1, "%s[%p]", Tag.c_str(), Data );
			BcProfiler::pImpl()->instantEvent( Buffer.data() );
		}
	}
};


#if PLATFORM_WINDOWS
#define PSY_PROFILE_FUNCTION							\
	BcProfilerSectionScope _ProfilerFunction_##__LINE__( __FUNCTION__ ) 
#else
#define PSY_PROFILE_FUNCTION							\
	BcProfilerSectionScope _ProfilerFunction_##__LINE__( __PRETTY_FUNCTION__ ) 
#endif

#define PSY_PROFILER_SECTION( _LocalName, _Tag ) BcProfilerSectionScope _LocalName##CPU( _Tag ) 
#define PSY_PROFILER_GPU_SECTION( _LocalName, _Tag ) BcProfilerGPUSectionScope _LocalName##GPU( _Tag ) 
#define PSY_PROFILER_START_ASYNC( _Tag, _Data ) BcProfilerStartAsync( _Tag, _Data )
#define PSY_PROFILER_STEP_ASYNC( _Tag, _Data ) BcProfilerStepAsync( _Tag, _Data )
#define PSY_PROFILER_FINISH_ASYNC( _Tag, _Data ) BcProfilerEndAsync( _Tag, _Data )
#define PSY_PROFILER_INSTANT_EVENT( _Tag, _Data ) BcProfilerInstantEvent( _Tag, _Data )	

#else

#define PSY_PROFILE_FUNCTION
#define PSY_PROFILER_SECTION( _LocalName, _Tag )
#define PSY_PROFILER_GPU_SECTION( _LocalName, _Tag )
#define PSY_PROFILER_START_ASYNC( _Tag, _Data )
#define PSY_PROFILER_STEP_ASYNC( _Tag, _Data )
#define PSY_PROFILER_FINISH_ASYNC( _Tag, _Data )
#define PSY_PROFILER_INSTANT_EVENT( _Tag, _Data )

#endif // PSY_USE_PROFILER

#endif // __BCPROFILER_H__