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

#include "System/SysFence.h"

#include <sstream>
#include <fstream>

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// Ctor
SysProfilerChromeTracing::SysProfilerChromeTracing()
{
	ProfilerSectionPool_.resize( 65536 );
	ProfilerSectionIndex_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysProfilerChromeTracing::~SysProfilerChromeTracing()
{

}

//////////////////////////////////////////////////////////////////////////
// registerThreadId
//virtual
void SysProfilerChromeTracing::beginProfiling()
{
	if( BeginCount_++ == 0 )
	{
		// Final flush out.
		SysFence Flush( BcErrorCode );

		// Reset allocation
		ProfilerSectionIndex_ = 0;

		// Mark timer!
		Timer_.mark();
		
		// And we're off!
		++ProfilingActive_;

		//
		SysFence Fence( BcErrorCode );
	}
	else
	{
		--BeginCount_;
	}
}

//////////////////////////////////////////////////////////////////////////
// registerThreadId
//virtual
void SysProfilerChromeTracing::endProfiling()
{
	if( --BeginCount_ == 0 )
	{	
		// Final flush out.
		SysFence Flush( BcErrorCode );

		// Stop profiling.
		--ProfilingActive_;

		// Wait for all workers to complete.
		SysFence Fence( BcErrorCode );
		SysFence Fence1( BcErrorCode );
		SysFence Fence2( BcErrorCode );
		SysFence Fence3( BcErrorCode );

		std::stringstream Stream;

		Stream << "{\"traceEvents\" : [";

		// Clear all per thread sections back down to the root nodes.
		for( BcU32 Idx = 0; Idx < ProfilerSectionIndex_; ++Idx )
		{
			TProfilerEvent* Event = &ProfilerSectionPool_[ Idx ];

			Stream << "{"
			       << "\"cat\": \"" << "Psybrus" << "\","
			       << "\"pid\": 0,"
		 		   << "\"tid\": " << Event->ThreadId_ << ","
			       << "\"ts\": " << Event->StartTime_ * 1000000.0 << ","
			       << "\"ph\": \"" << Event->Type_ << "\","
			       << "\"name\": \"" << Event->Tag_ << "\","
			       << "\"args\": {}"
			       << "},\n";
		}

		Stream << "{}]}";

		std::ofstream OutFileStream;
		OutFileStream.open( "test.json", std::ofstream::out | std::ofstream::trunc );
		OutFileStream << Stream.str();
		OutFileStream.close();
	}
	else
	{
		++BeginCount_;
	}
}

//////////////////////////////////////////////////////////////////////////
// enterSection
//virtual
void SysProfilerChromeTracing::enterSection( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent();
		if( Event != nullptr )
		{
			// Setup section.
			Event->Tag_ = Tag;
			Event->Type_ = "B";
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// exitSection
//virtual
void SysProfilerChromeTracing::exitSection( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent();
		if( Event != nullptr )
		{
			// Setup section.
			Event->Tag_ = Tag;
			Event->Type_ = "E";
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// startAsync
//virtual
void SysProfilerChromeTracing::startAsync( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent();
		if( Event != nullptr )
		{
			// Setup section.
			Event->Tag_ = Tag;
			Event->Type_ = "S";
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// endAsync
//virtual
void SysProfilerChromeTracing::endAsync( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent();
		if( Event != nullptr )
		{
			// Setup section.
			Event->Tag_ = Tag;
			Event->Type_ = "F";
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// instantEvent
//virtual
void SysProfilerChromeTracing::instantEvent( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerEvent* Event = allocEvent();
		if( Event != nullptr )
		{
			// Setup section.
			Event->Tag_ = Tag;
			Event->Type_ = "I";
			Event->ThreadId_ = BcCurrentThreadId();
			Event->StartTime_ = Timer_.time();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// allocEvent
SysProfilerChromeTracing::TProfilerEvent* SysProfilerChromeTracing::allocEvent()
{
	BcU32 Idx = ProfilerSectionIndex_++;
	if( Idx < ProfilerSectionPool_.size() )
	{
		auto Section = &ProfilerSectionPool_[ Idx ];
		*Section = TProfilerEvent();
		return Section;
	}
	else
	{
		// end and dump.
	}
	return nullptr;
}

#endif // PSY_USE_PROFILER
