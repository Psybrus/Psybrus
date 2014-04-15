#ifndef __REFLECTION_H__
#define __REFLECTION_H__

#include "Reflection/ReMacros.h"
#include "Reflection/ReUtility.h"
#include "Reflection/ReManager.h"
#include "Reflection/ReITypeSerialiser.h"
#include "Reflection/ReContainerAccessor.h"
#include "Reflection/ReContainerAccessorArray.h"
#include "Reflection/ReContainerAccessorList.h"
#include "Reflection/ReContainerAccessorMap.h"
#include "Reflection/ReContainerAccessorSet.h"
#include "Reflection/ReContainerAccessorVector.h"
#include "Reflection/ReObject.h"
#include "Reflection/ReObjectRef.h"
#include "Reflection/RePrimitive.h"
#include "Reflection/ReType.h"
#include "Reflection/ReEnumConstant.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReField.h"
#include "Reflection/ReClass.h"
#include "Reflection/ReClassSerialiser.h"
#include "Reflection/ReClassSerialiser_BasicType.h"
#include "Reflection/ReClassSerialiser_AbstractComplexType.h"
#include "Reflection/ReClassSerialiser_ComplexType.h"
#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClassSerialiser_BinaryDataType.h"
#include "Reflection/ReClassSerialiser_EnumType.h"
#include "Reflection/ReFieldAccessor.h"

//////////////////////////////////////////////////////////////////////////
// Functions
/**
	* @brief Construct Object
	* @param InClass Class to construct.
	* @param InName Name of class to construct.
	* @param InOwner Owning object.
	* @param InBasis Basis object to create it from.
	*/
Object* ConstructObject( const Class* InClass, const std::string& InName, Object* InOwner = nullptr, Object* InBasis = nullptr );

/**
	* @brief Construct Class
	* @param InName Name of class to construct.
	*/
template< class _Ty >
inline _Ty* ConstructClass( const std::string& InName )
{
	const Class* InClass = GetClass( InName );
	if( InClass != nullptr )
	{
		return InClass->construct< _Ty >();
	}
	return nullptr;
}

/**
	* @brief Construct Class
	*/
template< class _Ty >
inline _Ty* ConstructClass()
{
	const Class* InClass = _Ty::StaticGetClass();
	if( InClass != nullptr )
	{
		return InClass->construct< _Ty >();
	}
	return nullptr;
}

/**
	* @brief Construct Class
	*/
template< class _Ty >
inline _Ty* ConstructClass( const Class* InClass )
{
	if( InClass != nullptr )
	{
		return InClass->construct< _Ty >();
	}
	return nullptr;
}

/**
	* @brief Register a new class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @param Fields Array of fields to add to class.
	* @return Registered class.
	*/
template< typename _Class, std::size_t _Size >
Class* RegisterClass( const Field ( & Fields )[ _Size ], ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	Class->setFields( Fields );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	return Class;
}

/**
	* @brief Register a new class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @return Registered class.
	*/
template< typename _Class >
Class* RegisterClass( ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	return Class;
}
		
/**
	* @brief Register a new class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @param _Super Super type.
	* @param Fields Array of fields to add to class.
	* @return Registered class.
	*/
template< typename _Class, typename _Super, std::size_t _Size >
Class* RegisterClass( const Field ( & Fields )[ _Size ], ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	auto Super = GetClass( TypeTraits< _Super >::Name() );
	Class->setFields( Fields );
	Class->setSuper( Super );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	return Class;
}

/**
	* @brief Register a new class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @param _Super Super type.
	* @return Registered class.
	*/
template< typename _Class, typename _Super >
Class* RegisterClass( ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	auto Super = GetClass( TypeTraits< _Super >::Name() );
	Class->setSuper( Super );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	return Class;
}

/**
	* @brief Register a new abstract class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @param Fields Array of fields to add to class.
	* @return Registered class.
	*/
template< typename _Class, std::size_t _Size >
Class* RegisterAbstractClass( const Field ( & Fields )[ _Size ], ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	Class->setFields( Fields );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	return Class;
}

/**
	* @brief Register a new abstract class. Will get as usual, but setup what we need.
	* @param _Class Class type.
	* @param _Super Super type.
	* @param Fields Array of fields to add to class.
	* @return Registered class.
	*/
template< typename _Class, typename _Super, std::size_t _Size >
Class* RegisterAbstractClass( const Field ( & Fields )[ _Size ], ITypeSerialiser* Serialiser = nullptr )
{
	auto Class = GetClass( TypeTraits< _Class >::Name() );
	auto Super = GetClass( TypeTraits< _Super >::Name() );
	Class->setFields( Fields );
	Class->setSuper( Super );
	Class->setType< _Class >( Serialiser ? Serialiser : new ClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	return Class;
}

/**
	* @brief Register a new enum. Will get as usual, but setup what we need.
	* @param _Enum Enum type.
	* @param EnumConstants Array of enum constants to add to class.
	* @return Registered class.
	*/
template< typename _Enum, std::size_t _Size >
Enum* RegisterEnum( const EnumConstant ( & EnumConstants )[ _Size ] )
{
	auto Enum = GetEnum( TypeTraits< _Enum >::Name() );
	Enum->setConstants( EnumConstants, _Size );
	Enum->setType< _Enum >( new ClassSerialiser_EnumType( Enum->getName() )  );
	return Enum;
}

#endif
