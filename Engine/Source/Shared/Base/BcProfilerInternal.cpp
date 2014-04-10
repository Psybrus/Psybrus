/**************************************************************************
*
* File:		BcProfilerInternal.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Internal profiler implementation.
*			TODO: Move into sys.		
*
*
* 
**************************************************************************/

#include "Base/BcProfilerInternal.h"

#include "System/SysFence.h"

#include <sstream>
#include <fstream>

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// Ctor
BcProfilerInternal::BcProfilerInternal()
{
	ProfilerSectionPool_.resize( 65536 );
	ProfilerSectionIndex_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcProfilerInternal::~BcProfilerInternal()
{

}

//////////////////////////////////////////////////////////////////////////
// registerThreadId
//virtual
void BcProfilerInternal::beginProfiling()
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
void BcProfilerInternal::endProfiling()
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
void BcProfilerInternal::enterSection( const BcChar* Tag )
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
void BcProfilerInternal::exitSection( const BcChar* Tag )
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
void BcProfilerInternal::startAsync( const BcChar* Tag )
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
void BcProfilerInternal::endAsync( const BcChar* Tag )
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
// allocEvent
BcProfilerInternal::TProfilerEvent* BcProfilerInternal::allocEvent()
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
