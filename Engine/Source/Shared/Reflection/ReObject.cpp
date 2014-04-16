#include "Reflection/ReObject.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Object class definition
REFLECTION_DEFINE_BASE( ReObject );
	
void ReObject::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
#if REFLECTION_ENABLE_GC
		ReField( "RefCount_",			&ReObject::RefCount_,		bcRFF_TRANSIENT ),
#endif
		ReField( "Flags_",				&ReObject::Flags_ ),
		ReField( "Owner_",				&ReObject::Owner_ ),
		ReField( "Basis_",				&ReObject::Basis_ ),
		ReField( "Name_",				&ReObject::Name_ ),
	};
		
	ReRegisterClass< ReObject >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReObject::ReObject():
	Owner_( nullptr ),
	Basis_( nullptr )
{
	StaticAdd( this );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReObject::ReObject( ReNoInit ):
	Owner_( nullptr ),
	Basis_( nullptr )
{
	StaticAdd( this );
}
	
//////////////////////////////////////////////////////////////////////////
// Dtor
ReObject::~ReObject()
{
	// Remove from global object list.
	StaticRemove( this );

	// Handle destruction notification.
	if( Flags_ & (BcU32)ReObject::Flags::NotifyOnDeletion )
	{
		BcScopedLock< BcMutex > Lock( ObjectNotifyMutex_ );
		auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

		// If we find an entry, move list out, erase entry, and call all notifiers.
		if( ObjectNotifyListIt != ObjectNotifyMap_.end() )
		{
			auto ObjectNotifyList = std::move( ObjectNotifyListIt->second );

			for( auto ObjectNotify : ObjectNotifyList )
			{
				ObjectNotify->onObjectDeleted( this );
			}

			ObjectNotifyMap_.erase( ObjectNotifyListIt );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setName
void ReObject::setName( const std::string& Name )
{
	Name_ = Name;
}
	
//////////////////////////////////////////////////////////////////////////
// getName
const std::string& ReObject::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getFullName
std::string ReObject::getFullName() const
{
	const ReObject* Object = this;
	std::string Name = getName();
	while( Object->Owner_ != nullptr )
	{
		Object = Object->Owner_;
		Name = Object->getName() + "/" + Name;
	}
	return Name;
}

//////////////////////////////////////////////////////////////////////////
// addNotifier
void ReObject::addNotifier( ReIObjectNotify* ObjectNotify ) const
{
	// Add notifier flag so it knows to notify.
	Flags_ |= (BcU32)ReObject::Flags::NotifyOnDeletion;

	BcScopedLock< BcMutex > Lock( ObjectNotifyMutex_ );
	auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

	// If we find an entry, move list out, erase entry, and call all notifiers.
	if( ObjectNotifyListIt != ObjectNotifyMap_.end() )
	{
		ObjectNotifyListIt->second.push_back( ObjectNotify );
	}
	else
	{
		auto& ObjectNotifyListRet = ObjectNotifyMap_[ this ] = ObjectNotifyList( 1 );
		ObjectNotifyListRet.push_back( ObjectNotify );
	}
}
			
//////////////////////////////////////////////////////////////////////////
// removeNotifier
void ReObject::removeNotifier( ReIObjectNotify* ObjectNotify ) const
{
	BcAssertMsg( Flags_ & (BcU32)ReObject::Flags::NotifyOnDeletion, "Can't remove notifier from object that is flagged to not notify!" );

	BcScopedLock< BcMutex > Lock( ObjectNotifyMutex_ );
	auto ObjectNotifyListIt = ObjectNotifyMap_.find( this );

	BcAssertMsg( ObjectNotifyListIt != ObjectNotifyMap_.end(), "Can't remove notifier from object without a notification list." );

	ObjectNotifyListIt->second.remove( ObjectNotify );
}

//////////////////////////////////////////////////////////////////////////
// Statics
BcMutex ReObject::ObjectListMutex_;
ReObject::ObjectList ReObject::ObjectList_;
BcMutex ReObject::ObjectNotifyMutex_;
ReObject::ObjectNotifyMap ReObject::ObjectNotifyMap_;

//////////////////////////////////////////////////////////////////////////
// StaticAdd
//static
void ReObject::StaticAdd( ReObject* Object )
{
	BcScopedLock< BcMutex > Lock( ObjectListMutex_ );
	ObjectList_.push_back( Object );
}

//////////////////////////////////////////////////////////////////////////
// StaticRemove
//static
void ReObject::StaticRemove( ReObject* Object )
{
	BcScopedLock< BcMutex > Lock( ObjectListMutex_ );
	ObjectList_.remove( Object );
}


//////////////////////////////////////////////////////////////////////////
// StaticCollectGarbage
//static
void ReObject::StaticCollectGarbage()
{
	// This is mostly a functional placeholder before we experiment
	// with kicking off GC as a job so it can run in parallel to the
	// game.
#if REFLECTION_ENABLE_GC
	// Lock and grab a copy of the object list.
	ObjectListMutex_.lock();
	ObjectList ObjectList( ObjectList_ );
	ObjectListMutex_.unlock();

	// Iterate over all the objects, check their flags, and delete
	// if required.
	for( auto Object : ObjectList )
	{
		if( ( Object->Flags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) != 0 )
		{
			delete Object;
		}
	}
#endif
}
