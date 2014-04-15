#ifndef __REFLECTION_ENUM_H__
#define __REFLECTION_ENUM_H__

#include "Reflection/ReClass.h"

//////////////////////////////////////////////////////////////////////////
// Enum
class Enum:
	public Class
{
public:
	REFLECTION_DECLARE_DERIVED( Enum, Class );

public:
	Enum();
	Enum( const std::string& Name );
	virtual ~Enum(){};

	/**
		* @brief Set constants.
		* @param EnumConstants Constant array to add.
		* @param Elements Number of constants in array.
		*/
	void setConstants( const EnumConstant* EnumConstants, BcU32 Elements );

	/**
		* @brief Get enum constant
		* @param Value Value of enum.
		*/
	const EnumConstant* getEnumConstant( BcU32 Value );

	/**
		* @brief Get enum constant
		* @param Name Name of enum.
		*/
	const EnumConstant* getEnumConstant( const std::string& Name );
		
protected:
	std::vector< const EnumConstant* > EnumConstants_;
};

#endif
