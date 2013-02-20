/**************************************************************************
*
* File:		BcReflectionSerialise.cpp
* Author:	Neil Richardson
* Ver/Date:	26/12/12
* Description:
*			Reflection serialisation.
*
*
*
*
**************************************************************************/

#include "Base/BcReflectionSerialise.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcReflectionSerialise::BcReflectionSerialise()
{

}

//////////////////////////////////////////////////////////////////////////
// BcReflectionSerialise
//virtual
BcReflectionSerialise::~BcReflectionSerialise()
{

}
	
//////////////////////////////////////////////////////////////////////////
// serialise
//virtual
void BcReflectionSerialise::serialise( void* pData, const BcReflectionClass* pClass )
{
	serialiseClass( pData, pClass );
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
void BcReflectionSerialise::serialiseClass( void* pData, const BcReflectionClass* pClass )
{
	serialiseBeginClass( pData, pClass );
	serialiseClassFields( pData, pClass ); 
	serialiseEndClass( pData, pClass );
}

//////////////////////////////////////////////////////////////////////////
// serialiseBeginClass
//virtual
void BcReflectionSerialise::serialiseBeginClass( void* pData, const BcReflectionClass* pClass )
{

}

//////////////////////////////////////////////////////////////////////////
// serialiseEndClass
//virtual
void BcReflectionSerialise::serialiseEndClass( void* pData, const BcReflectionClass* pClass )
{

}

//////////////////////////////////////////////////////////////////////////
// serialiseClassFields
//virtual
void BcReflectionSerialise::serialiseClassFields( void* pData, const BcReflectionClass* pClass )
{
	const BcReflectionClass* pSuperClass = pClass->getSuper();
	
	// Serialise supers first.
	if( pSuperClass != NULL )
	{
		serialiseClassFields( pData, pSuperClass );
	}

	// Now serialise the fields.
	for( BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx )
	{
		const BcReflectionField* pField = pClass->getField( Idx );
		serialiseField( pData, pClass, pField );
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
void BcReflectionSerialise::serialiseField( void* pData, const BcReflectionClass* pParentClass, const BcReflectionField* pField )
{
	const BcReflectionType* pType = pField->getType();

	if( pType->isTypeOf< BcReflectionClass >() )
	{
		BcU32 PointerFlags = bcRFF_POINTER | bcRFF_REFERENCE;
		if( ( pField->getFlags() & PointerFlags ) == 0 )
		{
				const BcReflectionClass* pClass = static_cast< const BcReflectionClass* >( pType );
				void* pNewData = reinterpret_cast< BcU8* >( pData ) + pField->getOffset();
				serialiseClass( pNewData, pClass );
		}
		else
		{
			const BcReflectionClass* pClass = static_cast< const BcReflectionClass* >( pType );
			void* pPointerData = *pField->getData< void** >( pData );
			if( pPointerData != NULL )
			{
				serialiseClass( pPointerData, pClass );
			}
		}
	}
	
	if( pType == NULL )
	{
		BcPrintf( "%s %s::%s: \n", "Unknown", (*pParentClass->getName()).c_str(), (*pField->getName()).c_str() );
	}
}
