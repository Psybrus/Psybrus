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

#if 0
#include "BcReflectionSerialiseJson.h"

#include "BcFile.h"

//////////////////////////////////////////////////////////////////////////
// BcReflectionSerialisationJson_UnitTest
#include "Base/BcVec2d.h"
#include "Base/BcVec3d.h"
#include "Base/BcVec4d.h"
#include "Base/BcMat3d.h"
#include "Base/BcMat4d.h"
#include "Base/BcQuat.h"
#include "Base/BcMisc.h"

class BcReflectionSerialisationJson_UnitTest_Class
{
	REFLECTION_DECLARE_BASE( BcReflectionSerialisationJson_UnitTest_Class );

public:
	BcReflectionSerialisationJson_UnitTest_Class()
	{
		TestBcU32_ = 1337;
		TestBcVec4d_ = BcVec4d( 1.0f, 2.0f, 3.0f, 4.0f );
	}

	BcU32 TestBcU32_;
	BcVec4d TestBcVec4d_;
};

REFLECTION_DEFINE_BASE( BcReflectionSerialisationJson_UnitTest_Class );

BCREFLECTION_BASE_BEGIN( BcReflectionSerialisationJson_UnitTest_Class )
	BCREFLECTION_MEMBER( BcU32,							TestBcU32_,						bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcVec4d,						TestBcVec4d_,					bcRFF_DEFAULT )
BCREFLECTION_BASE_END();

void BcReflectionSerialisationJson_UnitTest()
{
	BcVec2d TestBcVec2d( 1.0f, 2.0f );
	BcVec3d TestBcVec3d( 1.0f, 2.0f, 3.0f );
	BcVec4d TestBcVec4d( 1.0f, 2.0f, 3.0f, 4.0f );
	BcReflectionSerialisationJson_UnitTest_Class TestClass;

	BcReflectionSerialisationJson_UnitTest_Class::StaticRegisterReflection();

	{
		BcReflectionSerialiseJson Serialiser( &TestBcVec2d, BcReflection::pImpl()->getClass( "BcVec2d" ) );
		Json::StyledWriter Writer;
		std::string Output = Writer.write(Serialiser.getRootValue());
		int a = 0; ++a;
	}
	
	{
		BcReflectionSerialiseJson Serialiser( &TestBcVec3d, BcReflection::pImpl()->getClass( "BcVec3d" ) );
		Json::StyledWriter Writer;
		std::string Output = Writer.write(Serialiser.getRootValue());
		int a = 0; ++a;
	}

	{
		BcReflectionSerialiseJson Serialiser( &TestBcVec4d, BcReflection::pImpl()->getClass( "BcVec4d" ) );
		Json::StyledWriter Writer;
		std::string Output = Writer.write(Serialiser.getRootValue());
		int a = 0; ++a;
	}

	{
		BcReflectionSerialiseJson Serialiser( &TestClass );
		Json::StyledWriter Writer;
		std::string Output = Writer.write(Serialiser.getRootValue());
		int a = 0; ++a;
	}
}

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
void BcReflectionSerialiseJson::serialiseBeginClass( void* pData, const ReClass* pClass )
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
void BcReflectionSerialiseJson::serialiseEndClass( void* pData, const ReClass* pClass )
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
void BcReflectionSerialiseJson::serialiseField( void* pData, const ReClass* pParentClass, const BcReflectionField* pField )
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
		// Default to an object value.
		MemberValue[ CurrMember ] = Json::Value( Json::objectValue );

		if( pField->getType()->isTypeOf< ReClass >() )
		{
			ValueStack_.push_back( pCurrValue_ );

			pCurrValue_ = &MemberValue[ CurrMember ];

			BcReflectionSerialise::serialiseField( pData, pParentClass, pField );

			pCurrValue_ = ValueStack_.back();
			ValueStack_.pop_back();
		}
		else
		{
			// Attempt to get the value out of it using native low level types.
			if( !getValue( pData, pField, MemberValue[ CurrMember ] ) )
			{
				BcBreakpoint;
			}
		}	
	}
}

//////////////////////////////////////////////////////////////////////////
// getValue
BcBool BcReflectionSerialiseJson::getValue( void* pData, const BcReflectionField* pField, Json::Value& Value )
{
	const BcReflectionType* pType = pField->getType();

	// 8 bit ints
	if( pType == &TYPE_BcU8 )
	{
		BcU8* pTypedData = pField->getData< BcU8 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcS8 )
	{
		BcS8* pTypedData = pField->getData< BcS8 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	// 16 bit ints
	else if( pType == &TYPE_BcU16 )
	{
		BcU16* pTypedData = pField->getData< BcU16 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcS16 )
	{
		BcS16* pTypedData = pField->getData< BcS16 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	// 32 bit ints
	else if( pType == &TYPE_BcU32 )
	{
		BcU32* pTypedData = pField->getData< BcU32 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcS32 )
	{
		BcS32* pTypedData = pField->getData< BcS32 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	// 64 bit
	else if( pType == &TYPE_BcU64 )
	{
		BcBreakpoint;
		Value = Json::Value();
		//BcU64* pTypedData = pField->getData< BcU64 >( pData );
		//return Json::Value( *pTypedData ); 
	}
	else if( pType == &TYPE_BcS64 )
	{
		BcBreakpoint;
		Value = Json::Value();
		//BcS64* pTypedData = pField->getData< BcS64 >( pData );
		//return Json::Value( *pTypedData ); 
	}
	// floats
	else if( pType == &TYPE_BcF32 )
	{
		BcF32* pTypedData = pField->getData< BcF32 >( pData );
		Value = Json::Value( *pTypedData ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcF64 )
	{
		BcF64* pTypedData = pField->getData< BcF64 >( pData );
		Value = Json::Value( *pTypedData );
		return BcTrue;
	}
	// char
	else if( pType == &TYPE_BcChar )
	{
		BcChar Data[] = 
		{
			*pField->getData< BcChar >( pData ),
			'\0'
		};
		Value = Json::Value( Data );
		return BcTrue;
	}
	else if( pType == &TYPE_BcBool )
	{
		BcBool* pTypedData = pField->getData< BcBool >( pData );
		Value = Json::Value( (bool)((*pTypedData) ? true : false) ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcHandle )
	{
		BcBreakpoint;
		Value = Json::Value(); 
	}
	else if( pType == &TYPE_BcSize )
	{
		BcBreakpoint;
		Value = Json::Value(); 
	}
	else if( pType == &TYPE_BcName )
	{
		BcName* pTypedData = pField->getData< BcName >( pData );
		Value = Json::Value( (**pTypedData).c_str() ); 
		return BcTrue;
	}
	else if( pType == &TYPE_BcHash )
	{
		BcHash* pTypedData = pField->getData< BcHash >( pData );
		Value = Json::Value( (BcU32)*pTypedData ); 
		return BcTrue;
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// setValue
BcBool BcReflectionSerialiseJson::setValue( void* pData, const BcReflectionField* pField, const Json::Value& Value )
{
	const BcReflectionType* pType = pField->getType();

	// 8 bit ints
	if( pType == &TYPE_BcU8 )
	{
		BcU8* pTypedData = pField->getData< BcU8 >( pData );
		*pTypedData = static_cast< BcU8 >( Value.asUInt() );
		return BcTrue;
	}
	else if( pType == &TYPE_BcS8 )
	{
		BcS8* pTypedData = pField->getData< BcS8 >( pData );
		*pTypedData = static_cast< BcS8 >( Value.asInt() );
		return BcTrue;
	}
	// 16 bit ints
	else if( pType == &TYPE_BcU16 )
	{
		BcU16* pTypedData = pField->getData< BcU16 >( pData );
		*pTypedData = static_cast< BcU16 >( Value.asUInt() );
		return BcTrue;
	}
	else if( pType == &TYPE_BcS16 )
	{
		BcS16* pTypedData = pField->getData< BcS16 >( pData );
		*pTypedData = static_cast< BcS16 >( Value.asInt() );
		return BcTrue;
	}
	// 32 bit ints
	else if( pType == &TYPE_BcU32 )
	{
		BcU32* pTypedData = pField->getData< BcU32 >( pData );
		*pTypedData = static_cast< BcU32 >( Value.asUInt() );
		return BcTrue;
	}
	else if( pType == &TYPE_BcS32 )
	{
		BcS32* pTypedData = pField->getData< BcS32 >( pData );
		*pTypedData = static_cast< BcU32 >( Value.asInt() );
		return BcTrue;
	}
	// 64 bit
	else if( pType == &TYPE_BcU64 )
	{
		BcBreakpoint;
	}
	else if( pType == &TYPE_BcS64 )
	{
		BcBreakpoint;
	}
	// floats
	else if( pType == &TYPE_BcF32 )
	{
		BcF32* pTypedData = pField->getData< BcF32 >( pData );
		*pTypedData = static_cast< BcF32 >( Value.asDouble() );
		return BcTrue;
	}
	else if( pType == &TYPE_BcF64 )
	{
		BcF64* pTypedData = pField->getData< BcF64 >( pData );
		*pTypedData = static_cast< BcF64 >( Value.asDouble() );
		return BcTrue;
	}
	// char
	else if( pType == &TYPE_BcChar )
	{
		BcChar* pTypedData = pField->getData< BcChar >( pData );
		*pTypedData = static_cast< BcChar >( Value.asCString()[ 0 ] );
		return BcTrue;
	}
	else if( pType == &TYPE_BcBool )
	{
		BcBool* pTypedData = pField->getData< BcBool >( pData );
		*pTypedData = Value.asBool() ? BcTrue : BcFalse;
		return BcTrue;
	}
	else if( pType == &TYPE_BcHandle )
	{
		BcBreakpoint;
	}
	else if( pType == &TYPE_BcSize )
	{
		BcBreakpoint;
	}
	else if( pType == &TYPE_BcName )
	{
		BcName* pTypedData = pField->getData< BcName >( pData );
		*pTypedData = BcName( Value.asCString() );
		return BcTrue;
	}
	else if( pType == &TYPE_BcHash )
	{
		BcHash* pTypedData = pField->getData< BcHash >( pData );
		*pTypedData = BcHash( static_cast< BcU32 >( Value.asUInt() ) );
		return BcTrue;
	}

	return BcFalse;
}
#endif