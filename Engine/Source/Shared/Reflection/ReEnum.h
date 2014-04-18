#ifndef __REFLECTION_ENUM_H__
#define __REFLECTION_ENUM_H__

#include "Reflection/ReClass.h"

//////////////////////////////////////////////////////////////////////////
// Enum
class ReEnum:
	public ReClass
{
public:
    REFLECTION_DECLARE_DERIVED( ReEnum, ReClass );

public:
    ReEnum();
    ReEnum( BcName Name );
    virtual ~ReEnum(){};

	/**
		* @brief Set constants.
		* @param EnumConstants Constant array to add.
		* @param Elements Number of constants in array.
		*/
    void setConstants( const ReEnumConstant* EnumConstants, BcU32 Elements );

	/**
		* @brief Get enum constant
		* @param Value Value of enum.
		*/
    const ReEnumConstant* getEnumConstant( BcU32 Value );

	/**
		* @brief Get enum constant
		* @param Name Name of enum.
		*/
    const ReEnumConstant* getEnumConstant( const std::string& Name );
		
protected:
    std::vector< const ReEnumConstant* > EnumConstants_;
};

#endif
