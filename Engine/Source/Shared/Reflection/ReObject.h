#ifndef __REFLECTION_OBJECT_H__
#define __REFLECTION_OBJECT_H__

#include "Reflection/ReManager.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"

#include "Reflection/ReObjectRef.h"
#include "Reflection/ReIObjectNotify.h"

//////////////////////////////////////////////////////////////////////////
/* @class Object
	* @brief Root reflection object with basic management.
	*
	* Non-POD and complex types can derive from this object. 
	* This gives basic garbage collection (through reference counting),
	* ownership support, naming, and is the basis for asset management.
	*
	* If you want to manually manage classes that can also be reflected,
	* and don't require custom serialisation, will not be serialised as
	* root objects, and do not have derived implementions when you store
	* pointers/references to the base class then you can likely get away
	* with not using this class for your basis, and it is recommended in
	* these cases.
	*
	* Overall recommendation is to use wisely - it is not a lightweight
	* class, and should be treated as a potential bottleneck or memory
	* consumer.
	*/
class ReObject
{
public:
	enum class Flags: BcU32
	{
		MarkedForDeletion = 0x00000001,						///!< Been marked for deletion.
		NotifyOnDeletion = 0x00000002,						///!< Object should notify on deletion.
	};

public:
    REFLECTION_DECLARE_BASE_MANUAL_NOINIT( ReObject );

public:
    ReObject();
    ReObject( ReNoInit );
    virtual ~ReObject();
	
	/**
		* @brief Set name.
		*/
	void							setName( const std::string& Name );

	/**
		* @brief Get object's name.
		*/
	const std::string&				getName() const;

	/**
		* @brief Get object's full name.
		*/
	std::string						getFullName() const;

	/**
		* @brief Get owner.
		*/
    ReObject*							getOwner() const;

	/**
		* @brief Set owner.
		*/
    void							setOwner( ReObject* Owner );

	/**
		* @brief Get root owner.
		*
		* Recurses down to find the root owner. Call infrequently.
		* A root owner should always have itself as it's owner.
		* An object with a null owner should be considered free
		* and unreferencable regarding serialisation for the
		* most part.
		*/
    ReObject*							getRootOwner() const;
					
	/**
		* @brief Set root owner.
		*
		* Recurses down to set the root owner.
		*/
    void							setRootOwner( ReObject* RootOwner );

	/**
		* @brief Get basis.
		*/
    ReObject*							getBasis() const;

	/**
		* @brief Increment ref count.
		*/
	BcU32								incRefCount() const;

	/**
		* @brief Decerement ref count.
		*/
	BcU32								decRefCount() const;

	/**
		* @brief Add notifier.
		*/
	void							addNotifier( ReIObjectNotify* ObjectNotify ) const;
			
	/**
		* @brief Remove notifier.
		*/
	void							removeNotifier( ReIObjectNotify* ObjectNotify ) const;

private:
    friend ReObject* ConstructObject( const ReClass* InClass, const std::string& InName, ReObject* InOwner, ReObject* InBasis );

	template< class _Ty > friend class ObjectRef;

	mutable BcAtomic< BcU32 >		RefCount_;			///!< Ref count.
	mutable BcAtomic< BcU32 >		Flags_;				///!< Flags.
    ReObject*							Owner_;					///!< Owner.
    ReObject*							Basis_;					///!< Object we're based upon.
	std::string						Name_;					///!< Name of object.

private:
    typedef std::list< ReObject* > ObjectList;
	typedef std::list< ReIObjectNotify* > ObjectNotifyList;
    typedef std::map< const ReObject*, ObjectNotifyList > ObjectNotifyMap;

	static BcMutex					ObjectListMutex_;		///!< Lock for object list. Access should be avoided.
	static ObjectList				ObjectList_;			///!< List of all active objects. Access should be avoided.

	static BcMutex					ObjectNotifyMutex_;		///!< Lock for object notify map.
	static ObjectNotifyMap			ObjectNotifyMap_;		///!< Map of objects to notify for.

	/**
		* Add object to the object list.
		*/
    static void						StaticAdd( ReObject* ReObject );

	/**
		* Remove object from object list.
		*/
    static void						StaticRemove( ReObject* ReObject );
public:

	/**
		* Perform basic garbage collection. Naive and simple.
		*/
	static void						StaticCollectGarbage();
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline ReObject* ReObject::getOwner() const
{
	return Owner_;
}

inline void ReObject::setOwner( ReObject* Owner )
{
		Owner_ = Owner;
}

inline ReObject* ReObject::getRootOwner() const
{
    ReObject* Owner = Owner_;
	while( Owner != nullptr && Owner != Owner->Owner_ )
	{
		Owner = Owner->Owner_;
	}
	return Owner;
}

inline void ReObject::setRootOwner( ReObject* RootOwner )
{
    ReObject* Owner = this;

	for(;;)
	{
		if( Owner->Owner_ == nullptr || Owner == Owner->Owner_ )
		{
			Owner->Owner_ = RootOwner;
			return;
		}
		else
		{
			Owner = Owner->Owner_;
		}
	}
}

inline ReObject* ReObject::getBasis() const
{
	return Basis_;
}

inline BcU32 ReObject::incRefCount() const
{
    BcAssert( ( Flags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) == 0 );
	return ++RefCount_;
}

inline BcU32 ReObject::decRefCount() const
{
    BcAssert( ( Flags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) == 0 );
	BcU32 RefCount;
	if( ( RefCount = --RefCount_ ) == 0 )
	{
        Flags_ |= (BcU32)ReObject::Flags::MarkedForDeletion;
	}
	return RefCount;
}

// Object reference inlines.
#include "Reflection/ReObjectRef.inl"

#endif // __OBJECT_H__
