#ifndef __REFLECTION_TYPE_H__
#define __REFLECTION_TYPE_H__

#include "Reflection/RePrimitive.h"

//////////////////////////////////////////////////////////////////////////
// Type
class Type:
	public Primitive
{
public:
	REFLECTION_DECLARE_DERIVED( Type, Primitive );

public:
	Type();
	virtual ~Type();

	/**
		* Set type.
		*/
	template< typename _Ty >
	void							setType( ITypeSerialiser* Serialiser )
	{
		Size_ = sizeof( _Ty );
		Serialiser_ = Serialiser;
	}

	/**
		* Set abstract type.
		*/
	template< typename _Ty >
	void							setAbstractType()
	{
		Size_ = sizeof( _Ty );
		Serialiser_ = nullptr;
	}

	/**
		* @brief Get Type Serialiser
		* @return
		*/
	inline const ITypeSerialiser*	getTypeSerialiser() const
	{
		return Serialiser_;
	}

	/**
		* Get size.
		*/
	BcU32								getSize() const;

protected:
	BcU32								Size_;
	ITypeSerialiser*				Serialiser_;

};

#endif
