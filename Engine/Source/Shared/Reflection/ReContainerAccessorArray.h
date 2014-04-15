#ifndef __REFLECTION_CONTAINERACCESSORARRAY_H__
#define __REFLECTION_CONTAINERACCESSORARRAY_H__

#include "Reflection/ReContainerAccessor.h"

//////////////////////////////////////////////////////////////////////////
// ArrayContainerAccessor
template< typename _Ty, size_t _Size >
class ArrayContainerAccessor:
	public ContainerAccessor
{
public:
	typedef typename TypeTraits< _Ty > ValueTraits;

public:
	class ArrayWriteIterator:
		public WriteIterator
	{
	public:
		ArrayWriteIterator( _Ty* pArrayData ):
			pArrayData_( pArrayData ),
			Index_( 0 )
		{

		}

		virtual ~ArrayWriteIterator()
		{

		}

		virtual void clear()
		{

		}

		virtual void add( void* pValue )
		{
			pArrayData_[ Index_++ ] = *reinterpret_cast< _Ty* >( pValue );
		}

		virtual void add( void* pKey, void* pValue )
		{
			BcAssertMsg( false, "ArrayContainerAccessor does not expose key." );
		}

		virtual bool isValid() const
		{
			return Index_ < _Size;
		}

	private:
		_Ty* pArrayData_;
		size_t Index_;
	};

	class ArrayReadIterator:
		public ReadIterator
	{
	public:
		ArrayReadIterator( _Ty* pArrayData ):
			pArrayData_( pArrayData ),
			Index_( 0 )
		{

		}

		virtual ~ArrayReadIterator()
		{

		}

		virtual void* getKey() const
		{
			return nullptr;
		}

		virtual void* getValue() const
		{
			return &pArrayData_[ Index_ ];
		}

		virtual void next()
		{
			++Index_;
		}

		virtual bool isValid() const
		{
			return Index_ < _Size;
		}

	private:
		_Ty* pArrayData_;
		size_t Index_;
	};

public:
	ArrayContainerAccessor()
	{
		pKeyType_ = nullptr;
		pValueType_ = GetClass< ValueTraits::Type >();
		KeyFlags_ = 0;
		ValueFlags_ = ValueTraits::Flags;
	}

	virtual ~ArrayContainerAccessor()
	{

	}

	virtual WriteIterator* newWriteIterator( void* pContainerData ) const
	{
		return new ArrayWriteIterator( reinterpret_cast< _Ty* >( pContainerData ) );
	}

	virtual ReadIterator* newReadIterator( void* pContainerData ) const
	{
		return new ArrayReadIterator( reinterpret_cast< _Ty* >( pContainerData ) );
	}
};

template < typename _Ty, size_t _Size >
ContainerAccessor* CreateContainerAccessor( _Ty ( & )[ _Size ], const Type*& pKeyType, const Type*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	typedef TypeTraits< _Ty > ValueTraits;
	pKeyType = nullptr;
	pValueType = GetClass< ValueTraits::Type >();
	KeyFlags = 0;
	ValueFlags = ValueTraits::Flags;
	return new ArrayContainerAccessor< _Ty, _Size >();
}
		
template < typename _Ty, size_t _Size >
ContainerAccessor* CreateContainerAccessor( std::array< _Ty, _Size >&, const Type*& pKeyType, const Type*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	typedef TypeTraits< _Ty > ValueTraits;
	pKeyType = nullptr;
	pValueType = GetClass< ValueTraits::Type >();
	KeyFlags = 0;
	ValueFlags = ValueTraits::Flags;
	return new ArrayContainerAccessor< _Ty, _Size >();
}

#endif
