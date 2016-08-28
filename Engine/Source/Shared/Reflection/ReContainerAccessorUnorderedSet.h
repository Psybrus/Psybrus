#ifndef __REFLECTION_CONTAINERACCESSORUNORDEREDSET_H__
#define __REFLECTION_CONTAINERACCESSORUNORDEREDSET_H__

#include "Reflection/ReContainerAccessor.h"

#include <unordered_set>

/////////////////////////////////////////////////////////////////////////
// ReUnorderedSetContainerAccessor
template < typename _Ty, typename _Comp, typename _Alloc >
class ReUnorderedSetContainerAccessor:
	public ReContainerAccessor
{
public:
    typedef ReTypeTraits< _Ty > ValueTraits;
	typedef std::unordered_set< _Ty, _Comp, _Alloc > Container;
	typedef typename Container::iterator ContainerIterator;
public:
	class SetWriteIterator:
		public WriteIterator
	{
	public:
		SetWriteIterator( Container& SetData ):
			SetData_( SetData )
		{

		}

		virtual ~SetWriteIterator()
		{

		}

		SetWriteIterator& operator = ( SetWriteIterator& Other )
		{
			SetData_ = Other.SetData_;
			return *this;
		}

		virtual void clear()
		{
			SetData_.clear();
		}

		template< typename _InternalTy >
		typename std::enable_if< std::is_copy_constructible< _InternalTy >::value >::type
		internalAdd( void* pValue )
		{
			SetData_.insert( *reinterpret_cast< _Ty* >( pValue ) );
		}

		template< typename _InternalTy >
		typename std::enable_if< !std::is_copy_constructible< _InternalTy >::value >::type
		internalAdd( void* pValue )
		{
		}

		virtual void add( void* pValue )
		{
			BcAssertMsg( std::is_copy_constructible< _Ty >::value, "_Ty is not trivially copyable." );
			internalAdd< _Ty >( pValue );
		}

		virtual void addMove( void* pValue )
		{
			BcAssertMsg( std::is_move_constructible< _Ty >::value, "_Ty is not move constructible." );
			SetData_.insert( std::move( *reinterpret_cast< _Ty* >( pValue ) ) );
		}

		virtual void add( void* pKey, void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose key." );
		}

		virtual void addMove( void* pKey, void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose key." );
		}

		virtual bool isValid() const
		{
			return true;
		}

	private:
		Container& SetData_;
	};

	class SetReadIterator:
		public ReadIterator
	{
	public:
		SetReadIterator( Container& SetData ):
			SetData_( SetData ),
			It_( SetData.begin() )
		{

		}

		virtual ~SetReadIterator()
		{

		}

		SetReadIterator& operator = ( SetReadIterator& Other )
		{
			SetData_ = Other.SetData_;
			It_ = Other.It_;
			return *this;
		}

		virtual void* getKey() const
		{
			return nullptr;
		}

		virtual void* getValue() const
		{
			return (void*)&(*It_);
		}

		virtual void next()
		{
			++It_;
		}

		virtual bool isValid() const
		{
			return It_ != SetData_.end();
		}

	private:
		Container& SetData_;
		ContainerIterator It_;
	};

public:
    ReUnorderedSetContainerAccessor()
	{
		pKeyType_ = nullptr;
        pValueType_ = ReManager::GetClass< 
        	typename ReTypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
        ValueFlags_ = ReTypeTraits< _Ty >::Flags;
	}

    virtual ~ReUnorderedSetContainerAccessor()
	{

	}

	virtual WriteIterator* newWriteIterator( void* pContainerData ) const
	{
		return new SetWriteIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}

	virtual ReadIterator* newReadIterator( void* pContainerData ) const
	{
		return new SetReadIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}
};

	
template < typename _Ty, typename _Comp, typename _Alloc >
ReContainerAccessor* CreateContainerAccessor( std::unordered_set< _Ty, _Comp, _Alloc >&, const ReClass*& pKeyType, const ReClass*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType =  nullptr;
    pValueType = ReManager::GetClass< 
    	typename ReTypeTraits< _Ty >::Type >();
	KeyFlags = 0;
    ValueFlags = ReTypeTraits< _Ty >::Flags;
    return new ReUnorderedSetContainerAccessor< _Ty, _Comp, _Alloc >();
}

#endif
