#ifndef __REFLECTION_CLASSSERIALISER_ABSTRACTCOMPLEXTYPE_H__
#define __REFLECTION_CLASSSERIALISER_ABSTRACTCOMPLEXTYPE_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_AbstractComplexType.
template < typename _Ty >
class ClassSerialiser_AbstractComplexType:
		public ClassSerialiser
{
public:
	ClassSerialiser_AbstractComplexType( const std::string& Name ): ClassSerialiser( Name ) {}
	virtual ~ClassSerialiser_AbstractComplexType() {}

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
