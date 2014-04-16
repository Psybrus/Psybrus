#ifndef __REFLECTION_MANAGER_H__
#define __REFLECTION_MANAGER_H__

#include "Reflection/ReUtility.h"

//////////////////////////////////////////////////////////////////////////
// Methods
class ReManager
{
public:
    /**
	* @brief Initialise.
	*/
    static void Init();

    /**
	* @brief Finalise.
	*/
    static void Fini();

    /**
	* @brief Get Type. Will not return if one does not exist.
	* @param InName Name of type.
	* @return
	*/
    static ReType* GetType( const std::string& InName );

    /**
	* @brief Get Class. Will always return, will add a new empty class if it does not exist.
	* @param InName Name of class.
	* @return
	*/
    static ReClass* GetClass( const std::string& InName );

    /**
	* @brief Get Enum. Will always return, will add a new empty enum if it does not exist.
	* @param InName Name of enum.
	* @return
	*/
    static ReEnum* GetEnum( const std::string& InName );

    /**
	* @brief Get Class. Will always return, will add a new empty class if it does not exist.
	* @param _Ty Type of class, autogens name.
	* @return
	*/
    template< typename _Ty >
    static ReClass* GetClass()
    {
        if( ReTypeTraits< _Ty >::IsEnum )
        {
            return ReManager::GetEnum( ReTypeTraits< _Ty >::Name() );
        }
        else
        {
            return ReManager::GetClass( ReTypeTraits< _Ty >::Name() );
        }
    }

    /**
	* @brief Get Classes of a base.
	* @return Pointer to class, or nullptr if out of range.
	*/
	static void GetClassesOfBase( std::vector< const ReClass* >& OutClasses, const ReClass* Base );

    /**
	* @brief Get Classes of a base.
	* @return Pointer to class, or nullptr if out of range.
	*/
    template< typename _Ty >
    static void GetClassesOfBase( std::vector< const ReClass* >& OutClasses )
    {
        GetClassesOfBase( OutClasses, _Ty::StaticGetClass() );
    }
};

#endif
