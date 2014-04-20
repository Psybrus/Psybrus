#ifndef __REFLECTION_TYPE_H__
#define __REFLECTION_TYPE_H__

#include "Reflection/ReAttributable.h"

//////////////////////////////////////////////////////////////////////////
// Type
class ReType:
	public ReAttributable
{
public:
    REFLECTION_DECLARE_DERIVED( ReType, ReAttributable );

public:
    ReType();
    virtual ~ReType();

	/**
		* Set type.
		*/
	template< typename _Ty >
	void							setType( ReITypeSerialiser* Serialiser )
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
	inline const ReITypeSerialiser*	getTypeSerialiser() const
	{
		return Serialiser_;
	}

	/**
		* Get size.
		*/
	BcU32								getSize() const;

protected:
	BcU32								Size_;
	ReITypeSerialiser*				Serialiser_;

};

#endif
