#ifndef __REFLECTION_CLASS_H__
#define __REFLECTION_CLASS_H__

#include "Reflection/ReType.h"
#include "Reflection/ReField.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// Class
class Class:
	public Type
{
public:
	REFLECTION_DECLARE_DERIVED( Class, Type );

public:
	Class();
	Class( const std::string& Name );
	virtual ~Class(){};

	/**
		* Set super.
		*/
	void							setSuper( const Class* Super );

	/**
		* Get super.
		*/
	const Class*					getSuper() const;

	/**
		* Have super class?
		*/
	BcBool							hasBaseClass( const Class* pClass ) const;

	/**
		* Set fields.
		*/
	void							setFields( BcU32 NoofFields, const Field* pFields );

	/**
		* Set fields by array.
		*/
	template< int _Size >
	void							setFields( const Field ( & Fields )[ _Size ] )
	{
		setFields( _Size, &Fields[ 0 ] );
	}

	/**
		* Get field.
		*/
	const Field*					getField( BcU32 Idx ) const;

	/**
		* Get noof fields.
		*/
	BcU32							getNoofFields() const;

	/**
		* Construct object.
		* @param pData Data to allocate into.
		*/
	template< class _Ty >
	_Ty*							construct( void* pData ) const
	{
		Serialiser_->construct( pData );
		return reinterpret_cast< _Ty* >( pData );
	}

	/**
		* Construct object with no init.
		* @param pData Data to allocate into.
		*/
	template< class _Ty >
	_Ty*							constructNoInit( void* pData ) const
	{
		BcMemZero( pData, getSize() );
		Serialiser_->constructNoInit( pData );
		return reinterpret_cast< _Ty* >( pData );
	}

	/**
		* Construct object.
		*/
	template< class _Ty >
	_Ty*							construct() const
	{
		return construct< _Ty >( BcMemAlign( getSize() ) );
	}

	/**
		* Construct object with no init.
		*/
	template< class _Ty >
	_Ty*							constructNoInit() const
	{
		return constructNoInit< _Ty >( BcMemAlign( getSize() ) );
	}

	/**
		* Destruct object.
		*/
	template< class _Ty >
	void							destruct( _Ty* pData ) const
	{
		Serialiser_->destruct( pData );
	}

protected:
	const Class*					Super_;
	std::vector< const Field* >		Fields_;
};

#endif
