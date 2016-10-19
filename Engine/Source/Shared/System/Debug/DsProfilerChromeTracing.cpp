/**************************************************************************
*
* File:		DsProfilerChromeTracing.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Chrome Tracing (chrome://tracing) profiler.
*			
*
*
* 
**************************************************************************/

#include "System/Debug/DsProfilerChromeTracing.h"

#include "System/SysKernel.h"

#include <sstream>
#include <fstream>

#if PLATFORM_ANDROID
#include <android_native_app_glue.h>
#include <android/native_activity.h>
#endif

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_PROFILER_ALLOCATOR_MB ( 32 )
#define MAX_EVENTS ( 1024 * 1024 )

//////////////////////////////////////////////////////////////////////////
// Ctor
DsProfilerChromeTracing::DsProfilerChromeTracing():
	Allocator_( 1024 * 1024 * MAX_PROFILER_ALLOCATOR_MB )
{
	ProfilerSections_.resize( MAX_EVENTS );
	BeginCount_ = 0;
	ProfilingActive_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsProfilerChromeTracing::~DsProfilerChromeTracing()
{

}

//////////////////////////////////////////////////////////////////////////
// setCurrentThreadName
//virtual
void DsProfilerChromeTracing::setCurrentThreadName( const char* Name )
{
	std::lock_guard< std::mutex > Lock( InternalMutex_ );
	ThreadNames_[ BcCurrentThreadId() ] = Name;
}

//////////////////////////////////////////////////////////////////////////
// initialiseGraphics
void DsProfilerChromeTracing::initialiseGraphics( const char* API, void* Context, void* Device )
{

}

//////////////////////////////////////////////////////////////////////////
// shutdownGraphics
void DsProfilerChromeTracing::shutdownGraphics()
{

}

//////////////////////////////////////////////////////////////////////////
// beginProfiling
//virtual
void DsProfilerChromeTracing::beginProfiling()
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
void DsProfilerChromeTracing::endProfiling()
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
		extern android_app* GAndroidApp;
		const char* InternalPath = GAndroidApp->activity->externalDataPath;

		std::array< char, 1024 > Buffer = { 0 };
		strftime( Buffer.data(), Buffer.size(), "chrome_tracing_%Y-%m-%d-%H-%M-%S.json", &LocalTime );
		BcSPrintf( FileName.data(), FileName.size(), "%s/%s", InternalPath, Buffer.data() );

#else
		strftime( FileName.data(), FileName.size(), "chrome_tracing_%Y-%m-%d-%H-%M-%S.json", &LocalTime );
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
void DsProfilerChromeTracing::enterSection( const char* Tag )
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
void DsProfilerChromeTracing::exitSection( const char* Tag )
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
// enterGPUSection
//virtual
void DsProfilerChromeTracing::enterGPUSection( const char* Tag )
{
}

//////////////////////////////////////////////////////////////////////////
// exitGPUSection
//virtual
void DsProfilerChromeTracing::exitGPUSection( const char* Tag )
{
}

//////////////////////////////////////////////////////////////////////////
// startAsync
//virtual
void DsProfilerChromeTracing::startAsync( const char* Tag, void* Data )
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
void DsProfilerChromeTracing::stepAsync( const char* Tag, void* Data )
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
void DsProfilerChromeTracing::endAsync( const char* Tag, void* Data )
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
void DsProfilerChromeTracing::instantEvent( const char* Tag )
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
DsProfilerChromeTracing::TProfilerEvent* DsProfilerChromeTracing::allocEvent( const char* Tag )
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
