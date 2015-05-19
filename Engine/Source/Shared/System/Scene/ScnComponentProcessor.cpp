#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnComponent.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// getTick
//static
BcF32 ScnComponentProcessFuncEntry::getTick()
{
	return SysKernel::pImpl()->getFrameTime();
}

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessor Reflection
REFLECTION_DEFINE_DERIVED( ScnComponentProcessor );

void ScnComponentProcessor::StaticRegisterClass()
{
	ReRegisterClass< ScnComponentProcessor, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentProcessor::ScnComponentProcessor()
{
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
