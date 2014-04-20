#ifndef __REFLECTION_CONTAINERACCESSORVECTOR_H__
#define __REFLECTION_CONTAINERACCESSORVECTOR_H__

#include "Reflection/ReContainerAccessor.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// VectorContainerAccessor
template < typename _Ty, typename _Alloc >
class ReVectorContainerAccessor:
	public ReContainerAccessor
{
public:
    typedef ReTypeTraits< _Ty > ValueTraits;
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
    ReVectorContainerAccessor()
	{
		pKeyType_ = nullptr;
        pValueType_ = ReManager::GetClass< ReTypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
        ValueFlags_ = ReTypeTraits< _Ty >::Flags;
	}

    virtual ~ReVectorContainerAccessor()
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
ReContainerAccessor* CreateContainerAccessor( std::vector< _Ty, _Alloc >&, const ReType*& pKeyType, const ReType*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType = nullptr;
    pValueType = ReManager::GetClass< ReTypeTraits< _Ty >::Type >();
	KeyFlags = 0;
    ValueFlags = ReTypeTraits< _Ty >::Flags;
    return new ReVectorContainerAccessor< _Ty, _Alloc >();
}

#endif
