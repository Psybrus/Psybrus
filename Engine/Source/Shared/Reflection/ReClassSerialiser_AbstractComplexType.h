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

	void construct( void* ) const override
	{
		BcBreakpoint;
	}

	void constructNoInit( void* ) const override
	{
		BcBreakpoint;
	}

	void destruct( void* ) const override
	{
		BcBreakpoint;
	}

	void* create() const override
	{
		BcBreakpoint;
		return nullptr;
	}

	void* createNoInit() const override
	{
		BcBreakpoint;
		return nullptr;
	}

	void destroy( void* ) const override
	{
		BcBreakpoint;
	}

	BcBool copy( void* pDst, void* pSrc ) const override
	{
		return false;
	}
};

#endif
