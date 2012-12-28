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
	pData_ = pData;

	serialiseBeginClass( pClass );
	serialiseClassFields( pClass ); 
	serialiseEndClass( pClass );
}

//////////////////////////////////////////////////////////////////////////
// serialiseBeginClass
//virtual
void BcReflectionSerialise::serialiseBeginClass( const BcReflectionClass* pClass )
{

}

//////////////////////////////////////////////////////////////////////////
// serialiseEndClass
//virtual
void BcReflectionSerialise::serialiseEndClass( const BcReflectionClass* pClass )
{

}

//////////////////////////////////////////////////////////////////////////
// serialiseClassFields
//virtual
void BcReflectionSerialise::serialiseClassFields( const BcReflectionClass* pClass )
{
	const BcReflectionClass* pSuperClass = pClass->getSuper();
	
	// Serialise supers first.
	if( pSuperClass != NULL )
	{
		serialiseClassFields( pSuperClass );
	}

	// Now serialise the fields.
	for( BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx )
	{
		const BcReflectionField* pField = pClass->getField( Idx );
		serialiseField( pClass, pField );
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
void BcReflectionSerialise::serialiseField( const BcReflectionClass* pParentClass, const BcReflectionField* pField )
{
	const BcReflectionType* pType = pField->getType();

	// Determine type for serialisation.
	if( pType != NULL )
	{
		BcPrintf( "%s %s::%s: \n", (*pType->getName()).c_str(), (*pParentClass->getName()).c_str(), (*pField->getName()).c_str() );
	}
	else
	{
		BcPrintf( "%s %s::%s: \n", "Unknown", (*pParentClass->getName()).c_str(), (*pField->getName()).c_str() );
	}
}
