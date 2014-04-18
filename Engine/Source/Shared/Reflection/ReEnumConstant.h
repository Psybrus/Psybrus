#ifndef __REFLECTION_ENUMCONSTANT_H__
#define __REFLECTION_ENUMCONSTANT_H__

#include "Reflection/RePrimitive.h"

//////////////////////////////////////////////////////////////////////////
// EnumConstant
class ReEnumConstant:
    public RePrimitive
{
public:
    REFLECTION_DECLARE_DERIVED( ReEnumConstant, RePrimitive );

public:
    ReEnumConstant();
    ReEnumConstant( BcName Name, BcU32 Value );

	template< typename _Ty >
	ReEnumConstant( BcName Name, _Ty Value )
	{
		setName( Name );
		Value_ = static_cast< BcU32 >( Value );
	}


    virtual ~ReEnumConstant(){};

	/**
		* Set value.
		*/
	void								setValue( BcU32 Value );

	/**
		* Get value.
		*/
	BcU32								getValue() const;

protected:
	BcU32								Value_;
};

#endif
