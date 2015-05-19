#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnComponent.h"

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
ScnComponentProcessor::ScnComponentProcessor( BcS32 Priority ):
	Priority_( Priority )
{
	ProcessFuncs_.reserve( 3 );
	ProcessFuncs_.emplace_back( 
		ScnComponentProcessFuncEntry(
			"Pre Update",
			ScnComponentPriority::DEFAULT_PRE_UPDATE + Priority_, 
			[]( const ScnComponentList& Components )
			{
				auto Tick = SysKernel::pImpl()->getFrameTime();
				for( auto Component : Components )
				{
					Component->preUpdate( Tick );
				}
			} ) );

	ProcessFuncs_.emplace_back(
		ScnComponentProcessFuncEntry(
			"Update",
			ScnComponentPriority::DEFAULT_UPDATE + Priority_,
			[]( const ScnComponentList& Components )
			{
				auto Tick = SysKernel::pImpl()->getFrameTime();
				for( auto Component : Components )
				{
					Component->update( Tick );
				}
			} ) );

	ProcessFuncs_.emplace_back(
		ScnComponentProcessFuncEntry(
			"Post Update",
			ScnComponentPriority::DEFAULT_POST_UPDATE + Priority_,
			[]( const ScnComponentList& Components )
			{
				auto Tick = SysKernel::pImpl()->getFrameTime();
				for( auto Component : Components )
				{
					Component->postUpdate( Tick );
				}
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentProcessor::ScnComponentProcessor( ScnComponentProcessFuncEntryInitialiserList ProcessFuncs ):
	ProcessFuncs_( ProcessFuncs )
{
}

//////////////////////////////////////////////////////////////////////////
// getProcessFuncs
//virtual
const ScnComponentProcessFuncEntryList& ScnComponentProcessor::getProcessFuncs()
{
	// Set class func should process.
	for( auto& ProcessFunc : ProcessFuncs_ )
	{
		ProcessFunc.Class_ = dynamic_cast< ReClass* >( getOwner() );
		BcAssert( ProcessFunc.Class_ != nullptr );
	}

	return ProcessFuncs_;
}
