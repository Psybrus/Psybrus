#include "ScnComponentProcessor.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessor Reflection
REFLECTION_DEFINE_DERIVED( ScnComponentProcessor );

void ScnComponentProcessor::StaticRegisterClass()
{
	ReRegisterClass< ScnComponentProcessor, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentProcessor::ScnComponentProcessor( BsS32 Priority ):
	Priority_( Priority )
{

}

//////////////////////////////////////////////////////////////////////////
// getProcessFuncs
//virtual
ScnComponentProcessFuncEntryList ScnComponentProcessor::getProcessFuncs()
{
	ScnComponentProcessFuncEntry EntryPreUpdate =
	{
		Priority_ - 10000, 
		[]( const ScnComponentList& Components )
		{
			auto Tick = SysKernel::pImpl()->getFrameTime();
			for( auto Component : Components )
			{
				Component->preUpdate( Tick );
			}
		}
	};

	ScnComponentProcessFuncEntry EntryUpdate =
	{
		Priority_,
		[]( const ScnComponentList& Components )
		{
			auto Tick = SysKernel::pImpl()->getFrameTime();
			for( auto Component : Components )
			{
				Component->update( Tick );
			}
		}
	};

	ScnComponentProcessFuncEntry EntryPostUpdate =
	{
		Priority_ + 10000,
		[]( const ScnComponentList& Components )
		{
			auto Tick = SysKernel::pImpl()->getFrameTime();
			for( auto Component : Components )
			{
				Component->postUpdate( Tick );
			}
		}
	};

	ScnComponentProcessFuncEntryList OutEntryList;
	OutEntryList.reserve( 3 );
	OutEntryList.push_back( EntryPreUpdate );
	OutEntryList.push_back( EntryUpdate );
	OutEntryList.push_back( EntryPostUpdate );
	return std::move( ScnComponentProcessFuncEntryList );
}
