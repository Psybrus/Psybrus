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
#include "Reflection/ReClass.h"
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
ReObject* ReConstructObject( 
	const ReClass* InClass, 
	const std::string& InName, 
	ReObject* InOwner = nullptr, 
	ReObject* InBasis = nullptr,
	std::function< void( ReObject* ) > postCreateFunc = nullptr );

/**
 * @brief Register a new class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param Fields Array of fields to add to class.
 * @return Registered class.
 */
template< typename _Class, std::size_t _Size >
ReClass& ReRegisterClass( ReField* ( & Fields )[ _Size ], ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	Class->setFields( Fields );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	BcAssert( Class->validate() );
	return *Class;
}

/**
 * @brief Register a new class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @return Registered class.
 */
template< typename _Class >
ReClass& ReRegisterClass( ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	BcAssert( Class->validate() );
	return *Class;
}
		
/**
 * @brief Register a new class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param _Super Super type.
 * @param Fields Array of fields to add to class.
 * @return Registered class.
 */
template< typename _Class, typename _Super, std::size_t _Size >
ReClass& ReRegisterClass( ReField* ( & Fields )[ _Size ], ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	auto Super = ReManager::GetClass( ReTypeTraits< _Super >::Name() );
	Class->setFields( Fields );
	Class->setSuper( Super );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	BcAssert( Class->validate() );
	return *Class;
}

/**
 * @brief Register a new class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param _Super Super type.
 * @return Registered class.
 */
template< typename _Class, typename _Super >
ReClass& ReRegisterClass( ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	auto Super = ReManager::GetClass( ReTypeTraits< _Super >::Name() );
	Class->setSuper( Super );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_ComplexType< _Class >( Class->getName() ) );
	BcAssert( Class->validate() );
	return *Class;
}

/**
 * @brief Register a new abstract class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @return Registered class.
 */
template< typename _Class >
ReClass& ReRegisterAbstractClass( ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	BcAssert( Class->validate() );
	return *Class;
}

/**
 * @brief Register a new abstract class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param _Super Super type.
 * @return Registered class.
 */
template< typename _Class, typename _Super >
ReClass& ReRegisterAbstractClass( ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	auto Super = ReManager::GetClass( ReTypeTraits< _Super >::Name() );
	Class->setSuper( Super );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	return *Class;
}

/**
 * @brief Register a new abstract class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param Fields Array of fields to add to class.
 * @return Registered class.
 */
template< typename _Class, std::size_t _Size >
ReClass& ReRegisterAbstractClass( ReField* ( & Fields )[ _Size ], ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	Class->setFields( Fields );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	return *Class;
}

/**
 * @brief Register a new abstract class. Will get as usual, but setup what we need.
 * @param _Class Class type.
 * @param _Super Super type.
 * @param Fields Array of fields to add to class.
 * @return Registered class.
 */
template< typename _Class, typename _Super, std::size_t _Size >
ReClass& ReRegisterAbstractClass( ReField* ( & Fields )[ _Size ], ReITypeSerialiser* Serialiser = nullptr )
{
	auto Class = ReManager::GetClass( ReTypeTraits< _Class >::Name() );
	auto Super = ReManager::GetClass( ReTypeTraits< _Super >::Name() );
	Class->setFields( Fields );
	Class->setSuper( Super );
	Class->template setType< _Class >( 
		Serialiser ? 
			Serialiser : 
			new ReClassSerialiser_AbstractComplexType< _Class >( Class->getName() ) );
	return *Class;
}

/**
 * @brief Register a new enum. Will get as usual, but setup what we need.
 * @param _Enum Enum type.
 * @param EnumConstants Array of enum constants to add to class.
 * @return Registered class.
 */
template< typename _Enum, std::size_t _Size >
ReEnum& ReRegisterEnum( ReEnumConstant* ( & EnumConstants )[ _Size ] )
{
	auto Enum = ReManager::GetEnum( ReTypeTraits< _Enum >::Name() );
	Enum->setConstants( &EnumConstants[ 0 ], _Size );
	Enum->template setType< _Enum >( 
		new ReClassSerialiser_EnumType( Enum->getName() ) );
	return *Enum;
}

#endif
