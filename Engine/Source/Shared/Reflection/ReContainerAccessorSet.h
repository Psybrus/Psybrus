#ifndef __REFLECTION_CONTAINERACCESSORSET_H__
#define __REFLECTION_CONTAINERACCESSORSET_H__

#include "Reflection/ReContainerAccessor.h"

#include <set>

/////////////////////////////////////////////////////////////////////////
// SetContainerAccessor
template < typename _Ty, typename _Comp, typename _Alloc >
class ReSetContainerAccessor:
	public ReContainerAccessor
{
public:
    typedef ReTypeTraits< _Ty > ValueTraits;
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

		SetWriteIterator& operator = ( SetWriteIterator& Other )
		{
			SetData_ = Other.SetData_;
			return *this;
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
    ReSetContainerAccessor()
	{
		pKeyType_ = nullptr;
        pValueType_ = ReManager::GetClass< 
        	typename ReTypeTraits< _Ty >::Type >();
		KeyFlags_ = 0;
        ValueFlags_ = ReTypeTraits< _Ty >::Flags;
	}

    virtual ~ReSetContainerAccessor()
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
ReContainerAccessor* CreateContainerAccessor( std::set< _Ty, _Comp, _Alloc >&, const ReClass*& pKeyType, const ReClass*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType =  nullptr;
    pValueType = ReManager::GetClass< 
    	typename ReTypeTraits< _Ty >::Type >();
	KeyFlags = 0;
    ValueFlags = ReTypeTraits< _Ty >::Flags;
    return new ReSetContainerAccessor< _Ty, _Comp, _Alloc >();
}

#endif
