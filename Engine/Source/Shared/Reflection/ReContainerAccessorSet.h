#ifndef __REFLECTION_CONTAINERACCESSORSET_H__
#define __REFLECTION_CONTAINERACCESSORSET_H__

#include "Reflection/ReContainerAccessor.h"

#include <set>

/////////////////////////////////////////////////////////////////////////
// SetContainerAccessor
template < typename _Ty, typename _Comp, typename _Alloc >
class SetContainerAccessor:
	public ContainerAccessor
{
public:
	typedef TypeTraits< _Ty > ValueTraits;
	typedef std::set< _Ty, _Comp, _Alloc > Container;
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

		virtual void clear()
		{
			SetData_.clear();
		}

		virtual void add( void* pValue )
		{
			SetData_.insert( *reinterpret_cast< _Ty* >( pValue ) );
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
	SetContainerAccessor()
	{
		pKeyType_ = nullptr;
		pValueType_ = GetClass< TypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
		ValueFlags_ = TypeTraits< _Ty >::Flags;
	}

	virtual ~SetContainerAccessor()
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
ContainerAccessor* CreateContainerAccessor( std::set< _Ty, _Comp, _Alloc >&, const Type*& pKeyType, const Type*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType =  nullptr;
	pValueType = GetClass< TypeTraits< _Ty >::Type >();
	KeyFlags = 0;
	ValueFlags = TypeTraits< _Ty >::Flags;
	return new SetContainerAccessor< _Ty, _Comp, _Alloc >();
}

#endif
