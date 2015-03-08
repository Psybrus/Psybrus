#ifndef __REFLECTION_CLASSSERIALISER_ABSTRACTCOMPLEXTYPE_H__
#define __REFLECTION_CLASSSERIALISER_ABSTRACTCOMPLEXTYPE_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_AbstractComplexType.
template < typename _Ty >
class ReClassSerialiser_AbstractComplexType:
		public ReClassSerialiser
{
public:
    ReClassSerialiser_AbstractComplexType( BcName Name ): ReClassSerialiser( Name ) {}
    virtual ~ReClassSerialiser_AbstractComplexType() {}

	virtual void construct( void* ) const
	{
		BcBreakpoint;
	}

	virtual void constructNoInit( void* ) const
	{
		BcBreakpoint;
	}

	virtual void destruct( void* ) const
	{
		BcBreakpoint;
	}

	virtual void* create() const
	{
		BcBreakpoint;
		return nullptr;
	}

	virtual void* createNoInit() const
	{
		BcBreakpoint;
		return nullptr;
	}

	virtual void destroy( void* ) const
	{
		BcBreakpoint;
	}

	virtual BcBool copy( void* pDst, void* pSrc ) const
	{
		return false;
	}
};

#endif
