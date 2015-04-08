#include "System/Network/NsReplication.h"
#include "System/Network/NsReplicationHandler.h"


//////////////////////////////////////////////////////////////////////////
// Ctor
NsReplication::NsReplication()
{
	ClassHandlerEntry NullEntry = { nullptr, nullptr };
	ClassHandlerList_.fill( NullEntry );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
NsReplication::~NsReplication()
{

}

//////////////////////////////////////////////////////////////////////////
// addObject
void NsReplication::addObject( ReObject* Object )
{
	const auto* Class = Object->getClass();
	auto ClassID = findClassID( Class );
}

//////////////////////////////////////////////////////////////////////////
// removeObject
void NsReplication::removeObject( ReObject* Object )
{

}

//////////////////////////////////////////////////////////////////////////
// addHandler
void NsReplication::addHandler( const ReClass* Class, class NsReplicationHandler* Handler )
{
	BcAssert( Class != nullptr );
	BcAssert( Handler != nullptr );

	// Verify it isn't added already.
#if PSY_DEBUG
	for( auto& ClassHandler : ClassHandlerList_ )
	{
		BcAssert( ClassHandler.Class_ != Class );
		BcAssert( ClassHandler.Handler_ != Handler );
	}
#endif

	// Find an empty slot in handler array.
	for( auto& ClassHandler : ClassHandlerList_ )
	{
		if( ClassHandler.Class_ == nullptr )
		{
			ClassHandler.Class_ = Class;
			ClassHandler.Handler_ = Handler;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// removeHandler
void NsReplication::removeHandler( const ReClass* Class, class NsReplicationHandler* Handler )
{
	BcAssert( Class != nullptr );
	BcAssert( Handler != nullptr );

	// Verify it has been added only once.
#if PSY_DEBUG
	BcBool Found = BcFalse;
	for( auto& ClassHandler : ClassHandlerList_ )
	{
		if( ClassHandler.Class_ == Class ||
			ClassHandler.Handler_ == Handler )
		{
			BcAssert( Found == BcFalse );
			Found = BcTrue;
		}
	}
	BcAssert( Found );
#endif

	// Remove handler.
	for( auto& ClassHandler : ClassHandlerList_ )
	{
		if( ClassHandler.Class_ == Class &&
			ClassHandler.Handler_ == Handler )
		{
			ClassHandler.Class_ = nullptr;
			ClassHandler.Handler_ = nullptr;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findClassID
NsReplicationClassID NsReplication::findClassID( const ReClass* Class ) const
{
	for( size_t Idx = 0; Idx < ClassHandlerList_.size(); ++Idx )
	{
		const auto& ClassHandler( ClassHandlerList_[ Idx ] );
		if( ClassHandler.Class_ == Class )
		{
			return static_cast< NsReplicationClassID >( Idx );
		}
	}
	BcBreakpoint;
	return std::numeric_limits< NsReplicationClassID >::max();
}


//////////////////////////////////////////////////////////////////////////
// update
void NsReplication::update()
{

}

//////////////////////////////////////////////////////////////////////////
// onMessageReceived
void NsReplication::onMessageReceived( const void* Data, size_t DataSize )
{

}
