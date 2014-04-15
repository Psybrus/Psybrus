#ifndef __REFLECTION_FIELD_H__
#define __REFLECTION_FIELD_H__

#include "Reflection/RePrimitive.h"
#include "Reflection/ReContainerAccessor.h"

//////////////////////////////////////////////////////////////////////////
// Field
class Field:
	public Primitive
{
public:
	REFLECTION_DECLARE_DERIVED( Field, Primitive );

public:
	Field();
	virtual ~Field()
	{
		delete ContainerAccessor_;
		ContainerAccessor_ = nullptr;
	}

	/**
		* Initialisation.
		*/
	template< typename _Class, typename _Ty >
	Field( const std::string& Name,
			_Ty( _Class::*field ),
			BcU32 Flags = 0 ):
		Type_( nullptr ),
		Offset_( 0 ),
		Flags_( 0 ),
		ContainerAccessor_( nullptr ),
		KeyType_( nullptr ),
		ValueType_( nullptr )
	{
		typedef TypeTraits< _Ty > LocalTypeTraits;
		setName( Name );
		setFlags( Flags | LocalTypeTraits::Flags );
		setOffset( offsetof( _Class, *field ) );
		ContainerAccessor_ = CreateContainerAccessor( ( ( _Class* ) 0 )->*field, KeyType_, ValueType_, KeyFlags_, ValueFlags_ );

		// If we get a container accessor, use the value type.
		if( ContainerAccessor_ == nullptr )
		{
			if( std::is_enum< _Ty >::value )
			{
				setType( GetEnum( LocalTypeTraits::Name() ) );
			}
			else
			{
				setType( GetClass( LocalTypeTraits::Name() ) );
			}
		}
		else
		{
			setType( ValueType_ );
		}
	}

	/**
		* Set type.
		*/
	void							setType( const Type* pType );

	/**
		* Get type.
		*/
	const Type*						getType() const;

	/**
		* Set offset.
		*/
	void							setOffset( BcU32 Offset );

	/**
		* Get offset.
		*/
	BcU32								getOffset() const;

	/**
		* Set flags.
		*/
	void							setFlags( BcU32 Flags );

	/**
		* Get flags.
		*/
	BcU32								getFlags() const;

	/**
		* Get data from source pointer.
		*/
	template< typename _Ty >
	_Ty*							getData( void* pObjectData ) const
	{
		// Handle if type is a simple deref pointer (* and &)
		if( ( Flags_ & bcRFF_SIMPLE_DEREF ) != 0 )
		{
			return *reinterpret_cast< _Ty** >( reinterpret_cast< BcU8* >( pObjectData ) + getOffset() );
		}
		else
		{
			return reinterpret_cast< _Ty* >( reinterpret_cast< BcU8* >( pObjectData ) + getOffset() );
		}
	}

	/**
		* @brief Is this field a container?
		*/
	bool							isContainer() const;

	/**
		* @brief Is this field a pointer type?
		*/
	bool							isPointerType() const;

	/**
		* Get type.
		*/
	const Type*						getKeyType() const;

	/**
		* Get type.
		*/
	const Type*						getValueType() const;

	/**
		* Get flags.
		*/
	BcU32								getKeyFlags() const;

	/**
		* Get flags.
		*/
	BcU32								getValueFlags() const;

	/**
		* @brief New write iterator. Returns nullptr if it's not a container. Owner must delete.
		*/
	ContainerAccessor::WriteIterator* newWriteIterator( void* pContainerData ) const;

	/**
		* @brief New read iterator. Returns nullptr if it's not a container. Owner must delete.
		*/
	ContainerAccessor::ReadIterator* newReadIterator( void* pContainerData ) const;

protected:
	BcU32								Offset_;
	const Type*						Type_;
	BcU32								Flags_;

	ContainerAccessor*				ContainerAccessor_;
	const Type*						KeyType_;
	const Type*						ValueType_;
	BcU32								KeyFlags_;
	BcU32								ValueFlags_;		
};

#endif
