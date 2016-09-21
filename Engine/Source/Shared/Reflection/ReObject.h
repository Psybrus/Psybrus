#ifndef __REFLECTION_OBJECT_H__
#define __REFLECTION_OBJECT_H__

#include "Reflection/ReManager.h"
#include <mutex>

#include "Base/BcName.h"

#include "Reflection/ReObjectRef.h"
#include "Reflection/ReIObjectNotify.h"

/**
 * Do we want garbage collection?
 */
#define REFLECTION_ENABLE_GC					( 0 )

/**
 * Do we want simple unique ID?
 */
#define REFLECTION_ENABLE_SIMPLE_UNIQUE_ID		( 1 )


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

private:
	ReObject( const ReObject& ){};
	ReObject( ReObject&& ){};

public:
    ReObject();
    ReObject( ReNoInit );
    virtual ~ReObject();
	
	/**
	 * @brief Set name.
	 */
	void							setName( BcName Name );

	/**
	 * @brief Get object's name.
	 */
	const BcName&					getName() const;
	
	/**
	 * @brief Set basis.
	 */
	void							setBasis( ReObject* Basis );

	/**
	 * @brief Get object's name.
	 */
	ReObject*						getBasis() const;

	/**
	 * @brief Get object's full name.
	 */
	std::string						getFullName() const;

	/**
	 * @brief Get owner.
	 */
    ReObject*						getOwner() const;

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
    ReObject*						getRootOwner() const;

#if REFLECTION_ENABLE_GC
	/**
	 * @brief Increment ref count.
	 */
	BcU32							incRefCount() const;

	/**
	 * @brief Decerement ref count.
	 */
	BcU32							decRefCount() const;
#endif

	/**
	 * @brief Add notifier.
	 */
	void							addNotifier( ReIObjectNotify* ObjectNotify ) const;
			
	/**
	 * @brief Remove notifier.
	 */
	void							removeNotifier( ReIObjectNotify* ObjectNotify ) const;

	/**
	 * @brief Adds flag for resource
	 */
	inline void						addObjectFlags( BcU32 Flag ) { ObjectFlags_ |= Flag; }

	/**
	 * @brief Removes flag for resource
	 */
	inline void						removeObjectFlags( BcU32 Flag ) { ObjectFlags_ = ObjectFlags_ & ~Flag; }
	
	/**
	 * @brief Gets flags for resource
	 */
	inline const BcU32				getObjectFlags() const { return ObjectFlags_; }

	/**
	 * Get unique identifier for resource
	 */
	inline const BcU32				getUniqueId() const
	{
#if REFLECTION_ENABLE_SIMPLE_UNIQUE_ID
		return UniqueId_;
#else
		return BcHash( this );
#endif
	}
private:
    friend ReObject* ReConstructObject( 
		const ReClass* InClass, 
		const std::string& InName, 
		ReObject* InOwner, 
		ReObject* InBasis );

	template< class _Ty, bool _IsWeak > friend class ReObjectRef;

#if REFLECTION_ENABLE_GC
	mutable std::atomic< BcU32 >		RefCount_;			///!< Ref count.
#endif
	mutable std::atomic< BcU32 >		ObjectFlags_;				///!< Flags.
    ReObject*							Owner_;				///!< Owner.
    ReObject*							Basis_;				///!< Object we're based upon.
	BcName								Name_;				///!< Name of object.

#if REFLECTION_ENABLE_SIMPLE_UNIQUE_ID
	BcU32								UniqueId_;
#endif

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
	 * Find object by unique id.
	 */
    static ReObject*				StaticFindByUniqueId( BcU32 UniqueId );

	/**
	 * Perform basic garbage collection. Naive and simple.
	 */
	static void						StaticCollectGarbage();
};

//////////////////////////////////////////////////////////////////////////
// Inlines
#if REFLECTION_ENABLE_GC

inline BcU32 ReObject::incRefCount() const
{
    BcAssert( ( ObjectFlags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) == 0 );
	return ++RefCount_;
}

inline BcU32 ReObject::decRefCount() const
{
    BcAssert( ( ObjectFlags_ & (BcU32)ReObject::Flags::MarkedForDeletion ) == 0 );
	BcU32 RefCount;
	if( ( RefCount = --RefCount_ ) == 0 )
	{
        ObjectFlags_ |= (BcU32)ReObject::Flags::MarkedForDeletion;
	}
	return RefCount;
}

#endif

// Object reference inlines.
#include "Reflection/ReObjectRef.inl"

#endif // __OBJECT_H__
