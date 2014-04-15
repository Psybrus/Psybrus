#include "Reflection/ReClassSerialiser_EnumType.h"
#include "Reflection/ReClass.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ClassSerialiser_EnumType::ClassSerialiser_EnumType( const std::string& Name ):
	ClassSerialiser( Name )
{
	Class_->setType< std::string >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ClassSerialiser_EnumType::~ClassSerialiser_EnumType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ClassSerialiser_EnumType::construct( void* pMemory ) const
{
	pMemory = BcMemAlign( Class_->getSize() );
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ClassSerialiser_EnumType::constructNoInit( void* pMemory ) const
{
	pMemory = BcMemAlign( Class_->getSize() );
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ClassSerialiser_EnumType::destruct( void* ) const
{
			
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
BcU32 ClassSerialiser_EnumType::getBinaryDataSize( void* pMemory ) const
{
	return Class_->getSize();
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ClassSerialiser_EnumType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
{
	switch( Class_->getSize() )
	{
	case 1:
		Serialiser << (BcU32)*((BcU8*)pInstance);
		break;
	case 2:
		Serialiser << (BcU32)*((BcU16*)pInstance);
		break;
	case 4:
		Serialiser << (BcU32)*((BcU32*)pInstance);
		break;
	default:
		BcAssert( false );
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromBinary
BcBool ClassSerialiser_EnumType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
{
	BcU32 Value;
	Serialiser >> Value;
	switch( Class_->getSize() )
	{
	case 1:
		*((BcU8*)pInstance) = (BcU8)Value;
		break;
	case 2:
		*((BcU16*)pInstance) = (BcU16)Value;
		break;
	case 4:
		*((BcU32*)pInstance) = (BcU32)Value;
		break;
	default:
		BcAssert( false );
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseToString
BcBool ClassSerialiser_EnumType::serialiseToString( const void* pInstance, std::string& OutString ) const
{
	BcU32 Value;
	switch( Class_->getSize() )
	{
	case 1:
		Value = *((BcU8*)pInstance);
		break;
	case 2:
		Value = *((BcU16*)pInstance);
		break;
	case 4:
		Value = *((BcU32*)pInstance);
		break;
	default:
		BcAssert( false );
		return false;
	}

	auto EnumConstant = static_cast< Enum* >( Class_ )->getEnumConstant( Value );
	if( EnumConstant != nullptr )
	{
		OutString = EnumConstant->getName();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ClassSerialiser_EnumType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	auto EnumConstant = static_cast< Enum* >( Class_ )->getEnumConstant( InString );

	if( EnumConstant != nullptr )
	{
		switch( Class_->getSize() )
		{
		case 1:
			*((BcU8*)pInstance) = (BcU8)EnumConstant->getValue();
			break;
		case 2:
			*((BcU16*)pInstance) = (BcU16)EnumConstant->getValue();
			break;
		case 4:
				*((BcU32*)pInstance) = (BcU32)EnumConstant->getValue();
			break;
		default:
			BcAssert( false );
			return false;
		}
	}		

	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ClassSerialiser_EnumType::copy( void* pDst, void* pSrc ) const
{
	BcU32& Dst = *reinterpret_cast< BcU32* >( pDst );
	BcU32& Src = *reinterpret_cast< BcU32* >( pSrc );
	Dst = Src;
	return true;
}
