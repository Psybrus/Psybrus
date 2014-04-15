#ifndef __REFLECTION_CONTAINERACCESSORVECTOR_H__
#define __REFLECTION_CONTAINERACCESSORVECTOR_H__

#include "Reflection/ReContainerAccessor.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// VectorContainerAccessor
template < typename _Ty, typename _Alloc >
class VectorContainerAccessor:
	public ContainerAccessor
{
public:
	typedef TypeTraits< _Ty > ValueTraits;
	typedef std::vector< _Ty, _Alloc > Container;

public:
	class VectorWriteIterator:
		public WriteIterator
	{
	public:
		VectorWriteIterator( Container& VectorData ):
			VectorData_( VectorData )
		{

		}

		virtual ~VectorWriteIterator()
		{

		}

		virtual void clear()
		{
			VectorData_.clear();
		}

		virtual void add( void* pValue )
		{
			VectorData_.push_back( *reinterpret_cast< _Ty* >( pValue ) );
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
		Container& VectorData_;
	};

	class VectorReadIterator:
		public ReadIterator
	{
	public:
		VectorReadIterator( Container& VectorData ):
			VectorData_( VectorData ),
			Index_( 0 )
		{

		}

		virtual ~VectorReadIterator()
		{

		}

		virtual void* getKey() const
		{
			return nullptr;
		}

		virtual void* getValue() const
		{
			return &VectorData_[ Index_ ];
		}

		virtual void next()
		{
			++Index_;
		}

		virtual bool isValid() const
		{
			return Index_ < VectorData_.size();
		}

	private:
		Container& VectorData_;
		size_t Index_;
	};

public:
	VectorContainerAccessor()
	{
		pKeyType_ = nullptr;
		pValueType_ = GetClass< TypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
		ValueFlags_ = TypeTraits< _Ty >::Flags;
	}

	virtual ~VectorContainerAccessor()
	{

	}

	virtual WriteIterator* newWriteIterator( void* pContainerData ) const
	{
		return new VectorWriteIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}

	virtual ReadIterator* newReadIterator( void* pContainerData ) const
	{
		return new VectorReadIterator( *reinterpret_cast< Container* >( pContainerData ) );
	}
};

	
template < typename _Ty, typename _Alloc >
ContainerAccessor* CreateContainerAccessor( std::vector< _Ty, _Alloc >&, const Type*& pKeyType, const Type*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType = nullptr;
	pValueType = GetClass< TypeTraits< _Ty >::Type >();
	KeyFlags = 0;
	ValueFlags = TypeTraits< _Ty >::Flags;
	return new VectorContainerAccessor< _Ty, _Alloc >();
}

#endif
