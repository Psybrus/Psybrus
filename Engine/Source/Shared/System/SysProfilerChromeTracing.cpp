/**************************************************************************
*
* File:		SysProfilerChromeTracing.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Chrome Tracing (chrome://tracing) profiler.
*			
*
*
* 
**************************************************************************/

#include "System/SysProfilerChromeTracing.h"

#include "System/SysKernel.h"

#include <sstream>
#include <fstream>

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_PROFILER_ALLOCATOR_MB ( 32 )
#define MAX_EVENTS ( 1024 * 1024 )

//////////////////////////////////////////////////////////////////////////
// Ctor
SysProfilerChromeTracing::SysProfilerChromeTracing():
	Allocator_( 1024 * 1024 * MAX_PROFILER_ALLOCATOR_MB )
{
	ProfilerSections_.resize( MAX_EVENTS );
	BeginCount_ = 0;
	ProfilingActive_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysProfilerChromeTracing::~SysProfilerChromeTracing()
{

}

//////////////////////////////////////////////////////////////////////////
// setThreadName
//virtual
void SysProfilerChromeTracing::setThreadName( BcThreadId ThreadId, const char* Name )
{
	std::lock_guard< std::mutex > Lock( InternalMutex_ );
	ThreadNames_[ ThreadId ] = Name;
}

//////////////////////////////////////////////////////////////////////////
// beginProfiling
//virtual
void SysProfilerChromeTracing::beginProfiling()
{
	if( BeginCount_++ == 0 )
	{
		// Final flush out.
		SysKernel::pImpl()->flushAllJobQueues();

		// Reset allocator.
		Allocator_.reset();
	
		// Reset sections.
		std::fill( ProfilerSections_.begin(), ProfilerSections_.end(), nullptr );
		ProfilerSectionIdx_ = 0;

		// Mark timer!
		Timer_.mark();
		
		// And we're off!
		ProfilingActive_ = 1;
	}
	else
	{
		--BeginCount_;
	}
}

//////////////////////////////////////////////////////////////////////////
// endProfiling
//virtual
void SysProfilerChromeTracing::endProfiling()
{
	if( --BeginCount_ == 0 )
	{	
		// Final flush out.
		SysKernel::pImpl()->flushAllJobQueues();

		// Stop profiling.
		ProfilingActive_ = 0;

		// Wait for all workers to complete.
		SysKernel::pImpl()->flushAllJobQueues();

		std::lock_guard< std::mutex > Lock( InternalMutex_ );

		auto Time = std::time( nullptr );
		auto LocalTime = *std::localtime( &Time );
		std::array< char, 1024 > FileName = { 0 };
#if PLATFORM_ANDROID
		strftime( FileName.data(), FileName.size() - 1, "/sdcard/Pictures/chrome_tracing_%Y-%m-%d-%H-%M-%S.json", &LocalTime );
#else
		strftime( FileName.data(), FileName.size() - 1, "chrome_tracing_%Y-%m-%d-%H-%M-%S.json", &LocalTime );
#endif

		std::array< char, 1024 > LineBuffer = { 0 };
		std::array< char, 32 > ID = { 0 };
		std::ofstream OutFileStream;

		OutFileStream.open( FileName.data(), std::ofstream::out | std::ofstream::trunc );
		OutFileStream << "{\"traceEvents\" : [\n";

		// Clear all per thread sections back down to the root nodes.
		for( BcU32 Idx = 0; Idx < ProfilerSections_.size(); ++Idx )
		{
			const TProfilerEvent* Event = ProfilerSections_[ Idx ];
			if( Event == nullptr )
			{
				break;
			}

#if PLATFORM_WINDOWS
			BcSPrintf( ID.data(), ID.size() - 1, "0x%p", Event->Data_ );
#else
			BcSPrintf( ID.data(), ID.size() - 1, "%p", Event->Data_ );
#endif

			auto ThreadNameIt = ThreadNames_.find( Event->ThreadId_ );
			if( ThreadNameIt != ThreadNames_.end() )
			{
				BcSPrintf( LineBuffer.data(), LineBuffer.size() - 1, 
					"\t{ \"cat\": \"%s\", \"pid\": %u, \"tid\": \"%s\", \"ts\": %.4f, \"ph\": \"%c\", \"name\": \"%s\", \"id\": \"%s\", \"args\": {} },\n",
					"", 0, ThreadNameIt->second.c_str(), Event->StartTime_ * 1000000.0, Event->Type_, Event->Tag_, ID.data() );
			}
			else
			{
				BcSPrintf( LineBuffer.data(), LineBuffer.size() - 1, 
					"\t{ \"cat\": \"%s\", \"pid\": %u, \"tid\": %u, \"ts\": %.4f, \"ph\": \"%c\", \"name\": \"%s\", \"id\": \"%s\", \"args\": {} },\n",
					"", 0, Event->ThreadId_, Event->StartTime_ * 1000000.0, Event->Type_, Event->Tag_, ID.data() );
			}
			OutFileStream << LineBuffer.data();
		}

		OutFileStream << "{}]}\n";
		OutFileStream.close();
		PSY_LOG( "Written profile to \"%s\"", FileName.data() );
	}
	else
	{
		++BeginCount_;
	}
}

//////////////////////////////////////////////////////////////////////////
// enterSection
//virtual
void SysProfilerChromeTracing::enterSection( const char* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 'B';
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// exitSection
//virtual
void SysProfilerChromeTracing::exitSection( const char* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 'E';
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// startAsync
//virtual
void SysProfilerChromeTracing::startAsync( const char* Tag, void* Data )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 's';
			Event->ThreadId_ = BcCurrentThreadId();
			Event->Data_ = Data;
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// stepAsync
//virtual
void SysProfilerChromeTracing::stepAsync( const char* Tag, void* Data )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 't';
			Event->ThreadId_ = BcCurrentThreadId();
			Event->Data_ = Data;
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// endAsync
//virtual
void SysProfilerChromeTracing::endAsync( const char* Tag, void* Data )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 'f';
			Event->Data_ = Data;
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// instantEvent
//virtual
void SysProfilerChromeTracing::instantEvent( const char* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent( Tag );
		if( Event != nullptr )
		{
			// Setup section.
			Event->Type_ = 'i';
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// allocEvent
SysProfilerChromeTracing::TProfilerEvent* SysProfilerChromeTracing::allocEvent( const char* Tag )
{
	auto Idx = ProfilerSectionIdx_++;

	if( Idx < ProfilerSections_.size() )
	{
		const size_t StringDataSize = BcStrLength( Tag ) + 1;
		void* EventData = Allocator_.allocate( sizeof( TProfilerEvent ) + StringDataSize );

		if( EventData != nullptr )
		{
			TProfilerEvent* ProfilerEvent = new ( EventData ) TProfilerEvent;		
			BcMemSet( ProfilerEvent->Tag_, 0, StringDataSize );
			BcStrCopy( ProfilerEvent->Tag_, BcU32( StringDataSize - 1 ), Tag );
			ProfilerSections_[ Idx ] = ProfilerEvent;
			return ProfilerEvent;
		}
	}
	return nullptr;
}

#endif // PSY_USE_PROFILER
