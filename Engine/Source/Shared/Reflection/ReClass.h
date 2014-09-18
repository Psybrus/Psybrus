#ifndef __REFLECTION_CLASS_H__
#define __REFLECTION_CLASS_H__

#include "Reflection/ReType.h"
#include "Reflection/ReField.h"
#include <vector>

//////////////////////////////////////////////////////////////////////////
// Class
class ReClass:
    public ReType
{
public:
    REFLECTION_DECLARE_DERIVED( ReClass, ReType );

public:
    ReClass();
    ReClass( BcName Name );
    virtual ~ReClass(){};

	/**
	 * Set super.
	 */
    void							setSuper( const ReClass* Super );

	/**
	 * Get super.
	 */
    const ReClass*					getSuper() const;

	/**
	 * Have super class?
	 */
    BcBool							hasBaseClass( const ReClass* pClass ) const;

	/**
	 * Set fields.
	 */
    void							setFields( ReFieldVector&& Fields );

	/**
	 * Set fields by array.
	 */
	template< int _Size >
    void							setFields( ReField* ( & Fields )[ _Size ] )
	{
		// Temporary until upgrade to VS2013 or above.
		ReFieldVector FieldVector( _Size );
		for( BcU32 Idx = 0; Idx < _Size; ++Idx )
		{
			FieldVector[ Idx ] = Fields[ Idx ];
		}
		setFields( std::move( FieldVector ) );
	}

	/*
	 * Get field.
	 */
    const ReField*					getField( BcU32 Idx ) const;

	/**
	 * Get noof fields.
	 */
	BcU32							getNoofFields() const;

	/**
	 * Get fields
	 */
	const ReFieldVector&			getFields() const;

	/**
	 * Validate.
	 */
	BcBool							validate() const;

	/**
	 * Construct object.
	 * @param pData Data to allocate into.
	 */
	template< class _Ty >
	_Ty*							construct( void* pData ) const
	{
		BcAssert( Serialiser_ );
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
		BcAssert( Serialiser_ );
		BcMemZero( pData, getSize() );
		Serialiser_->constructNoInit( pData );
		return reinterpret_cast< _Ty* >( pData );
	}

	/**
	 * Destruct object.
	 */
	template< class _Ty >
	void							destruct( _Ty* pData ) const
	{
		BcAssert( Serialiser_ );
		Serialiser_->destruct( pData );
	}

	/**
	 * Create object.
	 */
	template< class _Ty >
	_Ty*							create() const
	{
		BcAssert( Serialiser_ );
		return reinterpret_cast< _Ty* >( Serialiser_->create() );
	}

	/**
	 * Create object with no init.
	 */
	template< class _Ty >
	_Ty*							createNoInit() const
	{
		BcAssert( Serialiser_ );
		return reinterpret_cast< _Ty* >( Serialiser_->createNoInit() );
	}

	/**
	 * Destroy object.
	 */
	void							destroy( void* pData ) const;

protected:
    const ReClass* Super_;
    ReFieldVector Fields_;
};

#endif
