#include "Reflection/ReClassSerialiser_EnumType.h"
#include "Reflection/ReClass.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

#include <regex>

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
	
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ReClassSerialiser_EnumType::constructNoInit( void* pMemory ) const
{
	
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ReClassSerialiser_EnumType::destruct( void* ) const
{
			
}

//////////////////////////////////////////////////////////////////////////
// create
void* ReClassSerialiser_EnumType::create() const
{
	return createNoInit();
}
	
//////////////////////////////////////////////////////////////////////////
// createNoInit
void* ReClassSerialiser_EnumType::createNoInit() const
{
	switch( Class_->getSize() )
	{
	case 1:
		return new BcU8();
	case 2:
		return new BcU16();
	case 4:
		return new BcU32();
	default:
		BcAssert( false );
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ReClassSerialiser_EnumType::destroy( void* Object ) const
{
	switch( Class_->getSize() )
	{
	case 1:
		delete reinterpret_cast< BcU8* >( Object );
		break;
	case 2:
		delete reinterpret_cast< BcU16* >( Object );
		break;
	case 4:
		delete reinterpret_cast< BcU32* >( Object );
		break;
	default:
		BcAssert( false );
	}
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
size_t ReClassSerialiser_EnumType::getBinaryDataSize( const void* pMemory ) const
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

	auto* EnumConstant = static_cast< const ReEnum* >( Class_ )->getEnumConstant( Value );
	if( EnumConstant != nullptr )
	{
		OutString = *EnumConstant->getName();
	}
	else
	{
		// Reserve to reduce number of allocations.
		OutString.reserve( 1024 );

		// Attempt to find flags.
		bool PrependSeparator = false;
		for( BcU32 Idx = 0; Idx < 32; ++Idx )
		{
			BcU32 Flag = Value & ( 1 << Idx );
			if( Flag != 0 )
			{
				EnumConstant = static_cast< const ReEnum* >( Class_ )->getEnumConstant( Flag );
				if( EnumConstant != nullptr )
				{
					if( PrependSeparator )
					{
						OutString += "|";
					}
					OutString += *EnumConstant->getName();
					PrependSeparator = true;
				}
			}
		}

		// Shrink back to size required to converse memory.
		OutString.shrink_to_fit();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ReClassSerialiser_EnumType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	BcU32 Value = 0;
	auto* EnumConstant = static_cast< const ReEnum* >( Class_ )->getEnumConstant( InString );

	if( EnumConstant != nullptr )
	{
		Value = EnumConstant->getValue();
	}
	else
	{
		std::string Buffer;
		Buffer.reserve( InString.size() );
		
		for( size_t Idx = 0; Idx < InString.size(); ++Idx )
		{
			if( InString[ Idx ] != '|' )
			{
				Buffer += InString[ Idx ];
			}
			
			if( Buffer.size() > 0 && ( InString[ Idx ] == '|' || Idx == InString.size() - 1 ) )
			{	
				EnumConstant = static_cast< const ReEnum* >( Class_ )->getEnumConstant( Buffer );
				if( EnumConstant != nullptr )
				{
					Value |= EnumConstant->getValue();
				}
				
				Buffer.clear();
			}
		}
	}
	
	// Check size.
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
