/**************************************************************************
*
* File:		BcReflectionSerialise.h
* Author:	Neil Richardson
* Ver/Date:	26/12/12
* Description:
*			Reflection serialisation.
*
*
*
*
**************************************************************************/

#ifndef __BCREFLECTIONSERIALISE_H__
#define __BCREFLECTIONSERIALISE_H__

#include "Base/BcReflection.h"

//////////////////////////////////////////////////////////////////////////
// BcReflectionSerialise
class BcReflectionSerialise
{
public:
	BcReflectionSerialise();
	virtual ~BcReflectionSerialise();
	
	template< class _Ty >
	BcForceInline void serialise( _Ty* pBaseClass )
	{
		serialise( reinterpret_cast< void* >( pBaseClass ), pBaseClass->getClass() );
	}

	virtual void					serialise( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseClass( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseBeginClass( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseEndClass( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseClassFields( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseField( void* pData, const BcReflectionClass* pParentClass, const BcReflectionField* pField );
};

#endif
