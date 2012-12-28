/**************************************************************************
*
* File:		BcReflectionSerialiseJson.cpp
* Author:	Neil Richardson
* Ver/Date:	28/12/12
* Description:
*			Reflection serialisation.
*
*
*
*
**************************************************************************/

#include "BcReflectionSerialiseJson.h"

#include "BcFile.h"

//////////////////////////////////////////////////////////////////////////
// Strings
static const BcChar* GStringClass = "Class";
static const BcChar* GStringMembers = "Members";

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcReflectionSerialiseJson::~BcReflectionSerialiseJson()
{

}

//////////////////////////////////////////////////////////////////////////
// getRootValue
const Json::Value& BcReflectionSerialiseJson::getRootValue() const
{
	return RootValue_;
}

//////////////////////////////////////////////////////////////////////////
// serialiseBeginClass
//virtual
void BcReflectionSerialiseJson::serialiseBeginClass( void* pData, const BcReflectionClass* pClass )
{
	ValueStack_.push_back( pCurrValue_ );
	ClassStack_.push_back( CurrClass_ );
	CurrClass_ = pClass->getName();
	
	Json::Value& CurrValue( *pCurrValue_ );
	CurrValue[ GStringClass ] = (*pClass->getName()).c_str();
	CurrValue[ GStringMembers ] = Json::Value( Json::objectValue );

	pCurrValue_ = &CurrValue[ GStringMembers ];

	BcReflectionSerialise::serialiseBeginClass( pData, pClass );
}

//////////////////////////////////////////////////////////////////////////
// serialiseEndClass
//virtual
void BcReflectionSerialiseJson::serialiseEndClass( void* pData, const BcReflectionClass* pClass )
{
	pCurrValue_ = ValueStack_.back();
	CurrClass_ = ClassStack_.back();

	ValueStack_.pop_back();
	ClassStack_.pop_back();

	BcReflectionSerialise::serialiseEndClass( pData, pClass );
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
void BcReflectionSerialiseJson::serialiseField( void* pData, const BcReflectionClass* pParentClass, const BcReflectionField* pField )
{
	std::string CurrMember;
	// Differentiate super class's members.
	if( pParentClass->getName() == CurrClass_ )
	{
		CurrMember = (*pField->getName());
	}
	else
	{
		CurrMember = "_";
		CurrMember += (*pParentClass->getName());
		CurrMember += ":";
		CurrMember += (*pField->getName());
	}

	void* pActualData = pField->getData< void* >( pData );

	Json::Value& MemberValue( *pCurrValue_ );

	BcU32 IgnoreFlags = bcRFF_TRANSIENT;
	if( ( pField->getFlags() & IgnoreFlags ) == 0 )
	{
		if( !pField->getType()->isTypeOf< BcReflectionClass >() )
		{
			MemberValue[ CurrMember ] = getValue( pData, pField );
		}
		else
		{
			ValueStack_.push_back( pCurrValue_ );

			MemberValue[ CurrMember ] = Json::Value( Json::objectValue );
			pCurrValue_ = &MemberValue[ CurrMember ];

			BcReflectionSerialise::serialiseField( pData, pParentClass, pField );

			pCurrValue_ = ValueStack_.back();
			ValueStack_.pop_back();
		}	
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
Json::Value BcReflectionSerialiseJson::getValue( void* pData, const BcReflectionField* pField )
{
	const BcReflectionType* pType = pField->getType();

	// 8 bit ints
	if( pType == &TYPE_BcU8 )
	{
		BcU8* pTypedData = pField->getData< BcU8 >( pData );
		return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcS8 )
	{
		BcS8* pTypedData = pField->getData< BcS8 >( pData );
		return Json::Value( *pTypedData ); 
	}
	// 16 bit ints
	else if( pType == &TYPE_BcU16 )
	{
		BcU16* pTypedData = pField->getData< BcU16 >( pData );
		return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcS16 )
	{
		BcS16* pTypedData = pField->getData< BcS16 >( pData );
		return Json::Value( *pTypedData ); 
	}
	// 32 bit ints
	else if( pType == &TYPE_BcU32 )
	{
		BcU32* pTypedData = pField->getData< BcU32 >( pData );
		return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcS32 )
	{
		BcS32* pTypedData = pField->getData< BcS32 >( pData );
		return Json::Value( *pTypedData ); 
	}
	// 64 bit
	else if( pType == &TYPE_BcU64 )
	{
		BcBreakpoint;
		return Json::Value();
		//BcU64* pTypedData = pField->getData< BcU64 >( pData );
		//return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcS64 )
	{
		BcBreakpoint;
		return Json::Value();
		//BcS64* pTypedData = pField->getData< BcS64 >( pData );
		//return Json::Value( *pTypedData ); 
	}
	// floats
	else if( pType == &TYPE_BcF32 )
	{
		BcF32* pTypedData = pField->getData< BcF32 >( pData );
		return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcF64 )
	{
		BcF64* pTypedData = pField->getData< BcF64 >( pData );
		return Json::Value( *pTypedData ); 
	}
	// char
	else if( pType == &TYPE_BcChar )
	{
		BcChar Data[] = 
		{
			*pField->getData< BcChar >( pData ),
			'\0'
		};
		return Json::Value( Data ); 
	}
	else if( pType == &TYPE_BcBool )
	{
		BcBool* pTypedData = pField->getData< BcBool >( pData );
		return Json::Value( (bool)((*pTypedData) ? true : false) ); 
	}
	else if( pType == &TYPE_BcHandle )
	{
		BcBreakpoint;
		return Json::Value(); 
	}
	else if( pType == &TYPE_BcSize )
	{
		BcBreakpoint;
		return Json::Value(); 
	}
	else if( pType == &TYPE_BcName )
	{
		BcName* pTypedData = pField->getData< BcName >( pData );
		return Json::Value( (**pTypedData).c_str() ); 
	}
	else if( pType == &TYPE_BcHash )
	{
		BcHash* pTypedData = pField->getData< BcHash >( pData );
		return Json::Value( (BcU32)*pTypedData ); 
	}
	
	return Json::Value();
}
