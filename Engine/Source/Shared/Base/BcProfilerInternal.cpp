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

		// Setup thread profiler sections.
		PerThreadProfilerSections_.clear();
		for( const auto& Thread : Threads_ )
		{
			TProfilerSection* Section = allocSection();
			Section->Tag_ = Thread.Name_;
			Section->StartTime_ = Timer_.time();
			PerThreadProfilerSections_[ Thread.Id_ ] = Section;
		}

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
		for( auto& Thread : Threads_ )
		{
			TProfilerSection* RootSection = PerThreadProfilerSections_[ Thread.Id_ ];
			while( RootSection->Parent_ != nullptr )
			{
				RootSection = RootSection->Parent_;
			}

			// Stamp final time if non-zero.
			if( RootSection->EndTime_ == 0.0f )
			{
				RootSection->EndTime_ = Timer_.time();
			}

			PerThreadProfilerSections_[ Thread.Id_ ] = RootSection;

			// Dump section.
			Stream << dumpSection( &Thread, RootSection );
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
// registerThreadId
//virtual
void BcProfilerInternal::registerThreadId( BcThreadId Id, const BcChar* Name )
{
	TProfilerThread Thread = 
	{
		Name,
		Id
	};
	
	RegisterLock_.lock();
	Threads_.push_back( Thread );
	RegisterLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// enterSection
//virtual
void BcProfilerInternal::enterSection( const BcChar* Tag )
{
	if( ProfilingActive_ == 1 )
	{
		// New section.
		TProfilerSection* NewSection = allocSection();
		if( NewSection != nullptr )
		{
			// Grab thread id.
			BcThreadId Id = BcCurrentThreadId();
				
			// Grab old section.
			TProfilerSection* OldSection = PerThreadProfilerSections_[ Id ];
		
			// Setup section.
			NewSection->Tag_ = Tag;
			NewSection->StartTime_ = Timer_.time();

			// Insert into tree.
			NewSection->Parent_ = OldSection;
			NewSection->Next_ = OldSection->Child_;
			OldSection->Child_ = NewSection;
			PerThreadProfilerSections_[ Id ] = NewSection;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// exitSection
//virtual
void BcProfilerInternal::exitSection()
{
	if( ProfilingActive_ == 1 )
	{
		// Grab thread id.
		BcThreadId Id = BcCurrentThreadId();

		// Grab section.
		TProfilerSection* Section = PerThreadProfilerSections_[ Id ];

		// Stamp end time.
		Section->EndTime_ = Timer_.time();

		// Pop down a level if we can.
		if( Section->Parent_ != nullptr )
		{
			PerThreadProfilerSections_[ Id ] = Section->Parent_;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// allocSection
BcProfilerInternal::TProfilerSection* BcProfilerInternal::allocSection()
{
	BcU32 Idx = ProfilerSectionIndex_++;
	if( Idx < ProfilerSectionPool_.size() )
	{
		auto Section = &ProfilerSectionPool_[ Idx ];
		*Section = TProfilerSection();
		return Section;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// dumpSection
std::string BcProfilerInternal::dumpSection( TProfilerThread* Thread, TProfilerSection* Section )
{
	std::stringstream Stream;

	// Write begin event.
	Stream << "{"
	       << "\"cat\": \"" << Thread->Name_ << "\","
	       << "\"tid\": 0,"
		   << "\"pid\": " << Thread->Id_ << ","
	       << "\"ts\": " << Section->StartTime_ * 1000000.0 << ","
	       << "\"ph\": \"B\","
	       << "\"name\": \"" << Section->Tag_ << "\","
	       << "\"args\": {}"
	       << "},\n";

	// Dump children.
	TProfilerSection* Child = Section->Child_;
	while( Child != nullptr )
	{
		Stream << dumpSection( Thread, Child );
		Child = Child->Next_;
	}

	// Write end event.
	// Write begin event.
	Stream << "{"
	       << "\"cat\": \"" << Thread->Name_ << "\","
	       << "\"tid\": 0,"
		   << "\"pid\": " << Thread->Id_ << ","
	       << "\"ts\": " << Section->EndTime_ * 1000000.0 << ","
	       << "\"ph\": \"E\","
	       << "\"name\": \"" << Section->Tag_ << "\","
	       << "\"args\": {}"
	       << "},\n";

	return Stream.str();
}

#endif // PSY_USE_PROFILER
