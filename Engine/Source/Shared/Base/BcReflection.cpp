/**************************************************************************
*
* File:		BcReflection.cpp
* Author:	Neil Richardson
* Ver/Date:	24/12/12
* Description:
*			Reflection. Typing information + attributes.
*			Based mostly on Don Williamson's AltDevBlogADay article:
*			http://www.altdevblogaday.com/2011/09/25/reflection-in-c-part-1-introduction/
*
*
**************************************************************************/

#include "Base/BcReflection.h"

#include "Base/BcVectors.h"
#include "Base/BcMat3d.h"
#include "Base/BcMat4d.h"
#include "Base/BcQuat.h"
#include "Base/BcMisc.h"

// Base types must exist before anything else uses the database.
BcReflectionType TYPE_BcU8( "BcU8", sizeof( BcU8 ), BcReflectionConstruct< BcU8 >, BcReflectionDestruct< BcU8 > );
BcReflectionType TYPE_BcS8( "BcS8", sizeof( BcS8 ), BcReflectionConstruct< BcS8 >, BcReflectionDestruct< BcS8 > );
BcReflectionType TYPE_BcU16( "BcU16", sizeof( BcU16 ), BcReflectionConstruct< BcU16 >, BcReflectionDestruct< BcU16 > );
BcReflectionType TYPE_BcS16( "BcS16", sizeof( BcS16 ), BcReflectionConstruct< BcS16 >, BcReflectionDestruct< BcS16 > );
BcReflectionType TYPE_BcU32( "BcU32", sizeof( BcU32 ), BcReflectionConstruct< BcU32 >, BcReflectionDestruct< BcU32 > );
BcReflectionType TYPE_BcS32( "BcS32", sizeof( BcS32 ), BcReflectionConstruct< BcS32 >, BcReflectionDestruct< BcS32 > );
BcReflectionType TYPE_BcU64( "BcU64", sizeof( BcU64 ), BcReflectionConstruct< BcU64 >, BcReflectionDestruct< BcU64 > );
BcReflectionType TYPE_BcS64( "BcS64", sizeof( BcS64 ), BcReflectionConstruct< BcS64 >, BcReflectionDestruct< BcS64 > );
BcReflectionType TYPE_BcF32( "BcF32", sizeof( BcF32 ), BcReflectionConstruct< BcF32 >, BcReflectionDestruct< BcF32 > );
BcReflectionType TYPE_BcF64( "BcF64", sizeof( BcF64 ), BcReflectionConstruct< BcF64 >, BcReflectionDestruct< BcF64 > );
BcReflectionType TYPE_BcChar( "BcChar", sizeof( BcChar ), BcReflectionConstruct< BcChar >, BcReflectionDestruct< BcChar > );
BcReflectionType TYPE_BcBool( "BcBool", sizeof( BcBool ), BcReflectionConstruct< BcBool >, BcReflectionDestruct< BcBool > );
BcReflectionType TYPE_BcHandle( "BcHandle", sizeof( BcHandle ), BcReflectionConstruct< BcHandle >, BcReflectionDestruct< BcHandle > );
BcReflectionType TYPE_BcSize( "BcSize", sizeof( BcSize ), BcReflectionConstruct< BcSize >, BcReflectionDestruct< BcSize > );
BcReflectionType TYPE_BcName( "BcName", sizeof( BcName ), BcReflectionConstruct< BcName >, BcReflectionDestruct< BcName > );
BcReflectionType TYPE_BcHash( "BcHash", sizeof( BcHash ), BcReflectionConstruct< BcHash >, BcReflectionDestruct< BcHash > );


//////////////////////////////////////////////////////////////////////////
// RTTI for reflection primitives.
BCREFLECTION_DEFINE_BASE( BcReflectionPrimitive );
BCREFLECTION_DEFINE_DERIVED( BcReflectionType );
BCREFLECTION_DEFINE_DERIVED( BcReflectionEnumConstant );
BCREFLECTION_DEFINE_DERIVED( BcReflectionEnum );
BCREFLECTION_DEFINE_DERIVED( BcReflectionField );
BCREFLECTION_DEFINE_DERIVED( BcReflectionClass );

//////////////////////////////////////////////////////////////////////////
// BcReflectionPrimitive
BCREFLECTION_BASE_BEGIN( BcReflectionPrimitive )
	BCREFLECTION_MEMBER( BcName,						Name_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcHash,						NameHash_,						bcRFF_DEFAULT )
BCREFLECTION_BASE_END();

BcReflectionPrimitive::BcReflectionPrimitive( const BcName& Name ):
	Name_( Name ),
	NameHash_( (*Name).c_str() ) // Argh ditch this crazy std::string dep...
{

}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& BcReflectionPrimitive::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getNameHash
const BcHash& BcReflectionPrimitive::getNameHash() const
{
	return NameHash_;
}

//////////////////////////////////////////////////////////////////////////
// BcReflectionType
BCREFLECTION_DERIVED_BEGIN( BcReflectionPrimitive, BcReflectionType )
	BCREFLECTION_MEMBER( BcU32,							Size_,							bcRFF_DEFAULT )
BCREFLECTION_DERIVED_END();

BcReflectionType::BcReflectionType( const BcName& Name,
	                                BcU32 Size,
									BcReflectionConstructFunc constructFunc,
									BcReflectionDestructFunc destructFunc ):
	BcReflectionPrimitive( Name ),
	Size_( Size ),
	constructFunc_( constructFunc ),
	destructFunc_( destructFunc )
{

}

BcU32 BcReflectionType::getSize() const
{
	return Size_;
}

//////////////////////////////////////////////////////////////////////////
// BcReflectionEnumConstant
BCREFLECTION_DERIVED_BEGIN( BcReflectionPrimitive, BcReflectionEnumConstant )
	BCREFLECTION_MEMBER( BcU32,							Value_,							bcRFF_DEFAULT )
BCREFLECTION_DERIVED_END();

BcReflectionEnumConstant::BcReflectionEnumConstant( const BcName& Name, BcU32 Value ):
	BcReflectionPrimitive( Name ),
	Value_( Value )
{

}

//////////////////////////////////////////////////////////////////////////
// BcReflectionEnum
BCREFLECTION_DERIVED_BEGIN( BcReflectionType, BcReflectionEnum )
	BCREFLECTION_MEMBER( BcReflectionEnumConstant,		pEnumConstants_,				bcRFF_POINTER ),
	BCREFLECTION_MEMBER( BcU32,							NoofEnumConstants_,				bcRFF_DEFAULT )
BCREFLECTION_DERIVED_END();

BcReflectionEnum::BcReflectionEnum( const BcName& Name,
	                                BcU32 Size,
	                                BcReflectionConstructFunc constructFunc,
	                                BcReflectionDestructFunc destructFunc ):
	BcReflectionType( Name, Size, constructFunc, destructFunc )
{

}

//////////////////////////////////////////////////////////////////////////
// BcReflectionField
BCREFLECTION_DERIVED_BEGIN( BcReflectionPrimitive, BcReflectionField )
	BCREFLECTION_MEMBER( BcName,						Type_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcU32,							Offset_,						bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcU32,							Flags_,							bcRFF_DEFAULT )
BCREFLECTION_DERIVED_END();

BcReflectionField::BcReflectionField( const BcName& Name, const BcName& Type, BcU32 Offset, BcU32 Flags ):
	BcReflectionPrimitive( Name ),
	Type_( Type ),
	Offset_( Offset ),
	Flags_( Flags )
{

}

const BcReflectionType* BcReflectionField::getType() const
{
	return BcReflection::pImpl()->getType( Type_ );
}

BcU32 BcReflectionField::getOffset() const
{
	return Offset_;
}

BcU32 BcReflectionField::getFlags() const
{
	return Flags_;
}

//////////////////////////////////////////////////////////////////////////
// BcReflectionClass
BCREFLECTION_DERIVED_BEGIN( BcReflectionType, BcReflectionClass )
	BCREFLECTION_MEMBER( BcReflectionType,				Super_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcReflectionField,				pFields_,						bcRFF_POINTER ),
	BCREFLECTION_MEMBER( BcU32,							NoofFields_,					bcRFF_DEFAULT )
BCREFLECTION_DERIVED_END();

BcReflectionClass::BcReflectionClass( const BcName& Name,
                                      BcU32 Size,
                                      BcReflectionConstructFunc constructFunc,
                                      BcReflectionDestructFunc destructFunc,
                                      const BcName Super,
                                      const BcReflectionField* pFields,
                                      BcU32 NoofFields ):
 	BcReflectionType( Name, Size, constructFunc, destructFunc ),
	Super_( Super ),
	pFields_( pFields ),
	NoofFields_( NoofFields )
{

}

const BcReflectionClass* BcReflectionClass::getSuper() const
{
	return BcReflection::pImpl()->getClass( Super_ );
}

const BcBool BcReflectionClass::isTypeOfClass( const BcReflectionClass* pClass ) const
{
	if( this == pClass )
	{
		return BcTrue;
	}
	else
	{
		const BcReflectionClass* pSuperClass = getSuper();
		if( pSuperClass != NULL )
		{
			return pSuperClass->isTypeOfClass( pClass );
		}
	}
	return BcFalse;
}

const BcReflectionField* BcReflectionClass::getField( BcU32 Idx ) const
{
	BcAssert( Idx < NoofFields_ );
	return &pFields_[ Idx ];
}

BcU32 BcReflectionClass::getNoofFields() const
{
	return NoofFields_;
}

void BcReflectionClass::log() const
{
	BcPrintf( "BcReflectionClass: %s (%u)\n", (*Name_).c_str(), (BcU32)NameHash_ );
	BcPrintf( "Number of members: %u\n", NoofFields_ );
	for( BcU32 Idx = 0; Idx < NoofFields_; ++Idx )
	{
		const BcReflectionField& Field( pFields_[ Idx ] );
		BcPrintf( " - %s: Type %s, Offset %u\n", (*Field.getName()).c_str(), (*Field.getType()->getName()).c_str(), Field.getOffset() );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcReflection::BcReflection()
{
	addType( &TYPE_BcU8 );
	addType( &TYPE_BcS8 );
	addType( &TYPE_BcU16 );
	addType( &TYPE_BcS16 );
	addType( &TYPE_BcU32 );
	addType( &TYPE_BcS32 );
	addType( &TYPE_BcU64 );
	addType( &TYPE_BcS64 );
	addType( &TYPE_BcF32 );
	addType( &TYPE_BcF64 );
	addType( &TYPE_BcChar );
	addType( &TYPE_BcBool );
	addType( &TYPE_BcHandle );
	addType( &TYPE_BcSize );
	addType( &TYPE_BcName );
	addType( &TYPE_BcHash );

	// Vector types.
	static BcReflectionField Fields_BcVec2d[] =
	{
		BcReflectionField( "X_", "BcF32", 0 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Y_", "BcF32", 1 * sizeof( BcF32 ), bcRFF_DEFAULT )
	};
	
	static BcReflectionField Fields_BcVec3d[] =
	{
		BcReflectionField( "X_", "BcF32", 0 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Y_", "BcF32", 1 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Z_", "BcF32", 2 * sizeof( BcF32 ), bcRFF_DEFAULT )
	};

	static BcReflectionField Fields_BcVec4d[] =
	{
		BcReflectionField( "X_", "BcF32", 0 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Y_", "BcF32", 1 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Z_", "BcF32", 2 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "W_", "BcF32", 3 * sizeof( BcF32 ), bcRFF_DEFAULT )
	};

	static BcReflectionClass Class_BcVec2d( "BcVec2d", sizeof( BcVec2d ), BcReflectionConstruct< BcVec2d >, BcReflectionDestruct< BcVec2d >, BcName::NONE, Fields_BcVec2d, BcArraySize( Fields_BcVec2d ) );
	static BcReflectionClass Class_BcVec3d( "BcVec3d", sizeof( BcVec3d ), BcReflectionConstruct< BcVec3d >, BcReflectionDestruct< BcVec3d >, BcName::NONE, Fields_BcVec3d, BcArraySize( Fields_BcVec3d ) );
	static BcReflectionClass Class_BcVec4d( "BcVec4d", sizeof( BcVec4d ), BcReflectionConstruct< BcVec4d >, BcReflectionDestruct< BcVec4d >, BcName::NONE, Fields_BcVec4d, BcArraySize( Fields_BcVec4d ) );

	addType( &Class_BcVec2d );
	addType( &Class_BcVec3d );
	addType( &Class_BcVec4d );
	
	// Quaternion.
	static BcReflectionField Fields_BcQuat[] =
	{
		BcReflectionField( "X_", "BcF32", 0 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Y_", "BcF32", 1 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "Z_", "BcF32", 2 * sizeof( BcF32 ), bcRFF_DEFAULT ),
		BcReflectionField( "W_", "BcF32", 3 * sizeof( BcF32 ), bcRFF_DEFAULT )
	};

	static BcReflectionClass Class_BcQuat( "BcQuat", sizeof( BcQuat ), BcReflectionConstruct< BcQuat >, BcReflectionDestruct< BcQuat >, BcName::NONE, Fields_BcQuat, BcArraySize( Fields_BcQuat ) );

	addType( &Class_BcQuat );

	// Matrices.
	static BcReflectionField Fields_BcMat3d[] =
	{
		BcReflectionField( "Row0_", "BcVec3d", 0 * sizeof( BcVec3d ), bcRFF_DEFAULT ),
		BcReflectionField( "Row1_", "BcVec3d", 1 * sizeof( BcVec3d ), bcRFF_DEFAULT ),
		BcReflectionField( "Row2_", "BcVec3d", 2 * sizeof( BcVec3d ), bcRFF_DEFAULT )
	};

	static BcReflectionField Fields_BcMat4d[] =
	{
		BcReflectionField( "Row0_", "BcVec4d", 0 * sizeof( BcVec4d ), bcRFF_DEFAULT ),
		BcReflectionField( "Row1_", "BcVec4d", 1 * sizeof( BcVec4d ), bcRFF_DEFAULT ),
		BcReflectionField( "Row2_", "BcVec4d", 2 * sizeof( BcVec4d ), bcRFF_DEFAULT ),
		BcReflectionField( "Row3_", "BcVec4d", 3 * sizeof( BcVec4d ), bcRFF_DEFAULT )
	};

	static BcReflectionClass Class_BcMat3d( "BcMat3d", sizeof( BcMat3d ), BcReflectionConstruct< BcMat3d >, BcReflectionDestruct< BcMat3d >, BcName::NONE, Fields_BcMat3d, BcArraySize( Fields_BcMat3d ) );
	static BcReflectionClass Class_BcMat4d( "BcMat4d", sizeof( BcMat4d ), BcReflectionConstruct< BcMat4d >, BcReflectionDestruct< BcMat4d >, BcName::NONE, Fields_BcMat4d, BcArraySize( Fields_BcMat4d ) );

	addType( &Class_BcMat3d );
	addType( &Class_BcMat4d );

	// Internal types.
	BcReflectionPrimitive::StaticRegisterReflection();
	BcReflectionType::StaticRegisterReflection();
	BcReflectionEnumConstant::StaticRegisterReflection();
	BcReflectionEnum::StaticRegisterReflection();
	BcReflectionField::StaticRegisterReflection();
	BcReflectionClass::StaticRegisterReflection();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcReflection::~BcReflection()
{
}

//////////////////////////////////////////////////////////////////////////
// addType
void BcReflection::addType( const BcReflectionType* pType )
{
	BcAssert( BcIsGameThread() );
	TTypeMap::iterator It = TypeMap_.find( pType->getName() );
	BcAssertMsg( It == TypeMap_.end(), "BcReflection: Type already exists!" );
	TypeMap_[ pType->getName() ] = pType;
	TypeList_.push_back( pType );

	// Add to class list for quick look up.
	if( pType->isTypeOf< BcReflectionClass >() )
	{
		const BcReflectionClass* pClass = static_cast< const BcReflectionClass* >( pType );
		ClassMap_[ pClass->getName() ] = pClass;
		ClassList_.push_back( pClass );
	}
}

//////////////////////////////////////////////////////////////////////////
// getNoofTypes
BcU32 BcReflection::getNoofTypes()
{
	BcAssert( BcIsGameThread() );
	return static_cast< BcU32 >( TypeList_.size() );
}

//////////////////////////////////////////////////////////////////////////
// getType
const BcReflectionType* BcReflection::getType( BcU32 Idx )
{
	BcAssert( BcIsGameThread() );
	BcAssertMsg( Idx < TypeList_.size(), "BcReflection: Index out of range." );
	return TypeList_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getType
const BcReflectionType* BcReflection::getType( const BcName& Type )
{
	BcAssert( BcIsGameThread() );
	TTypeMap::iterator It = TypeMap_.find( Type );
	if( It != TypeMap_.end() )
	{
		return (*It).second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getNoofClasses
BcU32 BcReflection::getNoofClasses()
{
	BcAssert( BcIsGameThread() );
	return static_cast< BcU32 >( ClassList_.size() );
}

//////////////////////////////////////////////////////////////////////////
// getClass
const BcReflectionClass* BcReflection::getClass( BcU32 Idx )
{
	BcAssert( BcIsGameThread() );
	BcAssertMsg( Idx < ClassList_.size(), "BcReflection: Index out of range." );
	return ClassList_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getClass
const BcReflectionClass* BcReflection::getClass( const BcName& Class )
{
	BcAssert( BcIsGameThread() );
	TClassMap::iterator It = ClassMap_.find( Class );
	if( It != ClassMap_.end() )
	{
		return (*It).second;
	}

	return NULL;
}
