#ifndef __REFLECTION_CONTAINERACCESSORUNORDEREDMAP_H__
#define __REFLECTION_CONTAINERACCESSORUNORDEREDMAP_H__

#include "Reflection/ReContainerAccessor.h"

#include <unordered_map>

/////////////////////////////////////////////////////////////////////////
// ReUnorderedMapContainerAccessor
template < typename _Key, typename _Ty, typename _Pr, typename _Alloc >
class ReUnorderedMapContainerAccessor:
	public ReContainerAccessor
{
public:
    typedef ReTypeTraits< _Key > KeyTraits;
    typedef ReTypeTraits< _Ty > ValueTraits;
	typedef std::unordered_map< _Key, _Ty, _Pr, _Alloc > Container;
	typedef typename Container::iterator ContainerIterator;
public:
	class MapWriteIterator:
		public WriteIterator
	{
	public:
		MapWriteIterator( Container& MapData ):
			MapData_( MapData )
		{

		}

		virtual ~MapWriteIterator()
		{

		}

		MapWriteIterator& operator = ( MapWriteIterator& Other )
		{
			MapData_ = Other.MapData_;
			return *this;
		}

		virtual void clear()
		{
			MapData_.clear();
		}

		virtual void add( void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose only value." );
		}

		virtual void addMove( void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose only value." );
		}

		template< typename _InternalTy >
		typename std::enable_if< std::is_copy_constructible< _InternalTy >::value >::type
		internalAdd( void* pKey, void* pValue )
		{
			MapData_[ *reinterpret_cast< _Key* >( pKey ) ] = *reinterpret_cast< _Ty* >( pValue );
		}

		template< typename _InternalTy >
		typename std::enable_if< !std::is_copy_constructible< _InternalTy >::value >::type
		internalAdd( void* pValue )
		{
		}

		virtual void add( void* pKey, void* pValue )
		{
			BcAssertMsg( std::is_copy_constructible< _Ty >::value, "_Ty is not trivially copyable." );
			internalAdd< _Ty >( pKey, pValue );
		}

		virtual void addMove( void* pKey, void* pValue )
		{
			BcAssertMsg( std::is_move_constructible< _Ty >::value, "_Ty is not move constructible." );
			MapData_[ *reinterpret_cast< _Key* >( pKey ) ] = std::move( *reinterpret_cast< _Ty* >( pValue ) );
		}

		virtual bool isValid() const
		{
			return true;
		}

	private:
		Container& MapData_;
	};

	class MapReadIterator:
		public ReadIterator
	{
	public:
		MapReadIterator( Container& MapData ):
			MapData_( MapData ),
			It_( MapData.begin() )
		{

		}

		virtual ~MapReadIterator()
		{

		}

		MapReadIterator& operator = ( MapReadIterator& Other )
		{
			MapData_ = Other.MapData_;
			It_ = Other.It_;
			return *this;
		}

		virtual void* getKey() const
		{
			return (void*)&(It_->first);
		}

		virtual void* getValue() const
		{
			return &(It_->second);
		}

		virtual void next()
		{
			++It_;
		}

		virtual bool isValid() const
		{
			return It_ != MapData_.end();
		}

	private:
		Container& MapData_;
		ContainerIterator It_;
	};

public:
    ReUnorderedMapContainerAccessor()
	{
		pKeyType_ = ReManager::GetClass< 
			typename KeyTraits::Type >();
		pValueType_ = ReManager::GetClass< 
			typename ValueTraits::Type >();
		KeyFlags_ = KeyTraits::Flags;
		ValueFlags_ = ValueTraits::Flags;

		// We only support things serialisable to a string for now. This will possibly be improved later.
		BcAssertMsg( ( KeyFlags_ & bcRFF_ANY_POINTER_TYPE ) == 0, "MapContainerAccessor: Pointer type for key is unsupported yet." );
	}

    virtual ~ReUnorderedMapContainerAccessor()
	{

	}

	virtual WriteIterator* newWriteIterator( void* pContainerData ) const
	{
		return new MapWriteIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}

	virtual ReadIterator* newReadIterator( void* pContainerData ) const
	{
		return new MapReadIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}
};
	
template < typename _Key, typename _Ty, typename _Pr, typename _Alloc >
ReContainerAccessor* CreateContainerAccessor( std::unordered_map< _Key, _Ty, _Pr, _Alloc >&, const ReClass*& pKeyType, const ReClass*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
    pKeyType =  ReManager::GetClass< 
    	typename ReTypeTraits< _Key >::Type >();
    pValueType = ReManager::GetClass< 
    	typename ReTypeTraits< _Ty >::Type >();
    KeyFlags = ReTypeTraits< _Key >::Flags;
    ValueFlags = ReTypeTraits< _Ty >::Flags;
    return new ReUnorderedMapContainerAccessor< _Key, _Ty, _Pr, _Alloc >();
}

#endif
