#ifndef __REFLECTION_CONTAINERACCESSORLIST_H__
#define __REFLECTION_CONTAINERACCESSORLIST_H__

#include "Reflection/ReContainerAccessor.h"

#include <list>

//////////////////////////////////////////////////////////////////////////
// ListContainerAccessor
template < typename _Ty, typename _Alloc >
class ListContainerAccessor:
	public ContainerAccessor
{
public:
	typedef TypeTraits< _Ty > ValueTraits;
	typedef std::list< _Ty, _Alloc > Container;
	typedef typename Container::iterator ContainerIterator;

public:
	class ListWriteIterator:
		public WriteIterator
	{
	public:
		ListWriteIterator( Container& ListData ):
			ListData_( ListData )
		{

		}

		virtual ~ListWriteIterator()
		{

		}

		virtual void clear()
		{
			ListData_.clear();
		}

		virtual void add( void* pValue )
		{
			ListData_.push_back( *reinterpret_cast< _Ty* >( pValue ) );
		}

		virtual void add( void* pKey, void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose key." );
		}

		virtual bool isValid() const
		{
			return true;
		}

	private:
		Container& ListData_;
	};

	class ListReadIterator:
		public ReadIterator
	{
	public:
		ListReadIterator( Container& ListData ):
			ListData_( ListData ),
			It_( ListData.begin() )
		{

		}

		virtual ~ListReadIterator()
		{

		}

		virtual void* getKey() const
		{
			return nullptr;
		}

		virtual void* getValue() const
		{
			return &(*It_);
		}

		virtual void next()
		{
			++It_;
		}

		virtual bool isValid() const
		{
			return It_ != ListData_.end();
		}

	private:
		Container& ListData_;
		ContainerIterator It_;
	};

public:
	ListContainerAccessor()
	{
		pKeyType_ = nullptr;
		pValueType_ = GetClass< TypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
		ValueFlags_ = TypeTraits< _Ty >::Flags;
	}

	virtual ~ListContainerAccessor()
	{

	}

	virtual WriteIterator* newWriteIterator( void* pContainerData ) const
	{
		return new ListWriteIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}

	virtual ReadIterator* newReadIterator( void* pContainerData ) const
	{
		return new ListReadIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}
};

	
template < typename _Ty, typename _Alloc >
ContainerAccessor* CreateContainerAccessor( std::list< _Ty, _Alloc >&, const Type*& pKeyType, const Type*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType = nullptr;
	pValueType = GetClass< TypeTraits< _Ty >::Type >();
	KeyFlags = 0;
	ValueFlags = TypeTraits< _Ty >::Flags;
	return new ListContainerAccessor< _Ty, _Alloc >();
}

#endif
