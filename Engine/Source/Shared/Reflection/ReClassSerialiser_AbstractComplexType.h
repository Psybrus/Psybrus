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
		//BcAssertMsg( false, "Type is abstract." );
	}

	virtual void constructNoInit( void* ) const
	{
		//BcAssertMsg( false, "Type is abstract." );
	}

	virtual void destruct( void* ) const
	{
		//BcAssertMsg( false, "Type is abstract." );
	}

	virtual BcBool copy( void* pDst, void* pSrc ) const
	{
		return false;
	}

};

#endif
