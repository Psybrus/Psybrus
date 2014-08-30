#include "Reflection/ReClassSerialiser_EnumType.h"
#include "Reflection/ReClass.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ReClassSerialiser_EnumType::ReClassSerialiser_EnumType( BcName Name ):
	ReClassSerialiser( Name )
{
	Class_->setType< std::string >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ReClassSerialiser_EnumType::~ReClassSerialiser_EnumType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ReClassSerialiser_EnumType::construct( void* pMemory ) const
{
	pMemory = BcMemAlign( Class_->getSize() );
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ReClassSerialiser_EnumType::constructNoInit( void* pMemory ) const
{
	pMemory = BcMemAlign( Class_->getSize() );
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ReClassSerialiser_EnumType::destruct( void* ) const
{
			
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
BcU32 ReClassSerialiser_EnumType::getBinaryDataSize( void* pMemory ) const
{
	return Class_->getSize();
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ReClassSerialiser_EnumType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
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
BcBool ReClassSerialiser_EnumType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
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
BcBool ReClassSerialiser_EnumType::serialiseToString( const void* pInstance, std::string& OutString ) const
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

	auto EnumConstant = static_cast< ReEnum* >( Class_ )->getEnumConstant( Value );
	if( EnumConstant != nullptr )
	{
		OutString = *EnumConstant->getName();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ReClassSerialiser_EnumType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	auto EnumConstant = static_cast< ReEnum* >( Class_ )->getEnumConstant( InString );

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
BcBool ReClassSerialiser_EnumType::copy( void* pDst, void* pSrc ) const
{
	switch( Class_->getSize() )
	{
	case 1:
		{
			BcU8& Dst = *reinterpret_cast< BcU8* >( pDst );
			BcU8& Src = *reinterpret_cast< BcU8* >( pSrc );
			Dst = Src;
		}
		break;
	case 2:
		{
			BcU16& Dst = *reinterpret_cast< BcU16* >( pDst );
			BcU16& Src = *reinterpret_cast< BcU16* >( pSrc );
			Dst = Src;
		}
		break;
	case 4:
		{
			BcU32& Dst = *reinterpret_cast< BcU32* >( pDst );
			BcU32& Src = *reinterpret_cast< BcU32* >( pSrc );
			Dst = Src;
		}
		break;
	default:
		BcAssert( false );
		return false;
	}
	return true;
}
