#ifndef __REFLECTION_ENUMCONSTANT_H__
#define __REFLECTION_ENUMCONSTANT_H__

#include "Reflection/RePrimitive.h"

//////////////////////////////////////////////////////////////////////////
// EnumConstant
class EnumConstant:
	public Primitive
{
public:
	REFLECTION_DECLARE_DERIVED( EnumConstant, Primitive );

public:
	EnumConstant();
	EnumConstant( const std::string& Name, BcU32 Value );

	template< typename _Ty >
	EnumConstant( const std::string& Name, _Ty Value )
	{
		setName( Name );
		Value_ = static_cast< BcU32 >( Value );
	}


	virtual ~EnumConstant(){};

	/**
		* Set value.
		*/
	void							setValue( BcU32 Value );

	/**
		* Get value.
		*/
	BcU32								getValue() const;

protected:
	BcU32								Value_;
};

#endif
