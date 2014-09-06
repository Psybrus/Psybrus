#ifndef __REFLECTION_CONTAINERACCESSOR_H__
#define __REFLECTION_CONTAINERACCESSOR_H__

#include "Reflection/ReManager.h"

#include <memory>

//////////////////////////////////////////////////////////////////////////
// ContainerAccessor
class ReContainerAccessor
{
public:
	class WriteIterator
	{
	public:
		WriteIterator(){};
		virtual ~WriteIterator(){};

		virtual void clear() = 0;
		virtual void add( void* pValue ) = 0;
		virtual void add( void* pKey, void* pValue ) = 0;
		virtual bool isValid() const = 0;
	};

	class ReadIterator
	{
	public:
		ReadIterator(){};
		virtual ~ReadIterator(){};

		virtual void* getKey() const = 0;
		virtual void* getValue() const = 0;
		virtual void next() = 0;
		virtual bool isValid() const = 0;
	};

	typedef std::unique_ptr< WriteIterator > WriteIteratorUPtr;
	typedef std::unique_ptr< ReadIterator > ReadIteratorUPtr;

public:
    ReContainerAccessor()
	{

	}

    virtual ~ReContainerAccessor()
	{

	}

	/**
		* @brief Get a write iterator.
		* @return Write iterator.
		*/
	virtual WriteIterator* newWriteIterator( void* pContainerData ) const = 0;

	/**
		* @brief Get a read iterator.
		* @return Read iterator.
		*/
	virtual ReadIterator* newReadIterator( void* pContainerData ) const = 0;
		
protected:
    const ReType* pKeyType_;
    const ReType* pValueType_;

	BcU32 KeyFlags_;
	BcU32 ValueFlags_;
};

template < typename _Ty >
ReContainerAccessor* CreateContainerAccessor( _Ty&, const ReType*& pKeyType, const ReType*& pValueType, BcU32& KeyFlags, BcU32& ValueFlags )
{
	pKeyType = nullptr;
	pValueType = nullptr;
	KeyFlags = 0;
	ValueFlags = 0;
	return nullptr;
}

#endif
