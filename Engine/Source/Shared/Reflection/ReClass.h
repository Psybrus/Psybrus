#ifndef __REFLECTION_CLASS_H__
#define __REFLECTION_CLASS_H__

#include "Reflection/ReField.h"
#include "Reflection/ReITypeSerialiser.h"
#include <vector>

//////////////////////////////////////////////////////////////////////////
// Class
class ReClass:
	public ReAttributable
{
public:
    REFLECTION_DECLARE_DERIVED( ReClass, ReAttributable );

public:
	ReClass();
	ReClass( BcName Name );
	ReClass( const ReClass& ) = delete;
	virtual ~ReClass();

	/**
	 * Set type.
	 */
	template< typename _Ty >
	void setType( ReITypeSerialiser* Serialiser )
	{
		setFlags( ReTypeTraits< _Ty >::Flags );
		Size_ = sizeof( _Ty );
		Serialiser_ = Serialiser;
	}

	/**
	 * Set abstract type.
	 */
	template< typename _Ty >
	void setAbstractType()
	{
		setFlags( ReTypeTraits< _Ty >::Flags );
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
	size_t getSize() const;

	/**
	 * Set flags.
	 */
	void setFlags( BcU32 Flags );

	/**
	 * Get flags.
	 */
	BcU32 getFlags() const;

	/**
	 * Set super.
	 */
	void setSuper( const ReClass* Super );

	/**
	 * Get super.
	 */
	const ReClass* getSuper() const;

	/**
	 * Have super class?
	 */
	BcBool hasBaseClass( const ReClass* pClass ) const;

	/**
	 * Set fields.
	 */
	void setFields( ReFieldVector&& Fields );

	/**
	 * Set fields by array.
	 */
	template< int _Size >
	void setFields( ReField* ( & Fields )[ _Size ] )
	{
		// Temporary until upgrade to VS2013 or above.
		ReFieldVector FieldVector( _Size );
		for( BcU32 Idx = 0; Idx < _Size; ++Idx )
		{
			FieldVector[ Idx ] = Fields[ Idx ];
		}
		setFields( std::move( FieldVector ) );
	}

	/**
	 * Get field.
	 */
	const ReField* getField( size_t Idx ) const;

	/**
	 * Get noof fields.
	 */
	size_t getNoofFields() const;

	/**
	 * Get fields
	 */
	const ReFieldVector& getFields() const;

	/**
	 * Get a hash of this class.
	 */
	virtual BcU32 getHash() const;

	/**
	 * Validate.
	 */
	BcBool validate() const;

	/**
	 * Construct object.
	 * @param pData Data to allocate into.
	 */
	template< class _Ty >
	_Ty* construct( void* pData ) const
	{
		BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
		Serialiser_->construct( pData );
		return reinterpret_cast< _Ty* >( pData );
	}

	/**
	 * Construct object with no init.
	 * @param pData Data to allocate into.
	 */
	template< class _Ty >
	_Ty* constructNoInit( void* pData ) const
	{
		BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
		BcMemZero( pData, getSize() );
		Serialiser_->constructNoInit( pData );
		return reinterpret_cast< _Ty* >( pData );
	}

	/**
	 * Destruct object.
	 */
	template< class _Ty >
	void destruct( _Ty* pData ) const
	{
		BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
		Serialiser_->destruct( pData );
	}

	/**
	 * Create object.
	 */
	template< class _Ty >
	_Ty* create() const
	{
		BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
		return reinterpret_cast< _Ty* >( Serialiser_->create() );
	}

	/**
	 * Create object with no init.
	 */
	template< class _Ty >
	_Ty* createNoInit() const
	{
		BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
		return reinterpret_cast< _Ty* >( Serialiser_->createNoInit() );
	}

	/**
	 * Destroy object.
	 */
	void destroy( void* pData ) const;

protected:
	/// Custom type serialiser.
	ReITypeSerialiser* Serialiser_;

	/// Class super.
	const ReClass* Super_;

	/// Total size of class in bytes.
	size_t Size_;

	/// Flags associated with class.
	BcU32 ClassFlags_;

	/// Fields in class.
	ReFieldVector Fields_;
};

#endif
