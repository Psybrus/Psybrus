#ifndef __REFLECTION_MANAGER_H__
#define __REFLECTION_MANAGER_H__

#include "Reflection/ReUtility.h"

//////////////////////////////////////////////////////////////////////////
// Methods
/**
	* @brief Initialise.
	*/
void Init();

/**
	* @brief Finalise.
	*/
void Fini();

/**
	* @brief Get Type. Will not return if one does not exist.
	* @param InName Name of type.
	* @return
	*/
Type* GetType( const std::string& InName );

/**
	* @brief Get Class. Will always return, will add a new empty class if it does not exist.
	* @param InName Name of class.
	* @return
	*/
Class* GetClass( const std::string& InName );

/**
	* @brief Get Enum. Will always return, will add a new empty enum if it does not exist.
	* @param InName Name of enum.
	* @return
	*/
Enum* GetEnum( const std::string& InName );

/**
	* @brief Get Class. Will always return, will add a new empty class if it does not exist.
	* @param _Ty Type of class, autogens name.
	* @return
	*/
template< typename _Ty >
Class* GetClass()
{
	if( TypeTraits< _Ty >::IsEnum )
	{
		return GetEnum( TypeTraits< _Ty >::Name() );
	}
	else
	{
		return GetClass( TypeTraits< _Ty >::Name() );
	}
}

/**
	* @brief Get Classes of a base.
	* @return Pointer to class, or nullptr if out of range.
	*/
void GetClassesOfBase( std::vector< const Class* >& OutClasses, const Class* Base );

/**
	* @brief Get Classes of a base.
	* @return Pointer to class, or nullptr if out of range.
	*/
template< typename _Ty >
void GetClassesOfBase( std::vector< const Class* >& OutClasses )
{
	GetClassesOfBase( OutClasses, _Ty::StaticGetClass() );
}

#endif
