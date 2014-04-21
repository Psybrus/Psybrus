#ifndef __REFLECTION_FIELDACCESSOR_H__
#define __REFLECTION_FIELDACCESSOR_H__

#include "Reflection/ReManager.h"
#include "Reflection/ReField.h"
#include "Reflection/ReClass.h"
#include "Reflection/ReObject.h"

//////////////////////////////////////////////////////////////////////////
/* @brief FieldAccessor
	* 
	* Utility class to simplify the access to fields and their data.
	* It can be cumbersome, and there are a lot of repeated patterns in
	* serialisation that we don't want to bloat the field class with,
	* which is where this comes in.
	*/
class ReFieldAccessor
{
public:
	/**
		* @brief Initialise field accessor.
		* @param ParentObjectData Object the field is in.
		* @param Field Field.
		*/
    inline ReFieldAccessor( void* ParentObjectData, const ReField* Field ):
		Data_( ParentObjectData ),
		Field_( Field )
	{
		BcAssert( Field_->getType()->isTypeOf< ReClass >() );
	}

	/**
		* @brief Initialise field accessor from another field accessor, but with another parent.
		* @param ParentObjectData Object the field is in.
		* @param OtherFieldAccessor Other field accessor.
		*/
    inline ReFieldAccessor( void* ParentObjectData, const ReFieldAccessor& OtherFieldAccessor ):
		Data_( ParentObjectData ),
		Field_( OtherFieldAccessor.Field_ )
	{
		BcAssert( Field_->getType()->isTypeOf< ReClass >() );
	}
				

	/**
		* @brief Get raw data pointer.
		* Will not dereference or do any funky stuff. This will point straight at the field.
		*/
	inline void* getRawData() const
	{
		return reinterpret_cast< BcU8* >( Data_ ) + Field_->getOffset();
	}

	/**
		* @brief Set data pointer.
		* @pre Must be a pointer type to set.
		* Handled object reference count too.
		*/
	inline void setData( void* Data ) const
	{
		BcAssert( Field_->isPointerType() );
		void** DataRef = reinterpret_cast< void** >( getRawData() );
		*DataRef = Data;

		// Increment reference count if we are an object reference.
		if( *DataRef != nullptr && ( Field_->getFlags() & bcRFF_OBJECT_REFERENCE ) != 0 )
		{
#if REFLECTION_ENABLE_GC
			reinterpret_cast< ReObject* >( *DataRef )->incRefCount();
#endif
		}
	}

	/**
		* @brief Copy from data.
		* If it's a pointer type, it expects there to be an object set already.
		* @return If it's successfully copied.
		*/
	inline BcBool copy( void* Data ) const
	{
		const ReClass* FieldClass = static_cast< const ReClass* >( Field_->getType() );
		if( Field_->isPointerType() )
		{
			BcAssert( getData() != nullptr );
		}
		return FieldClass->getTypeSerialiser()->copy( getData(), Data );
	}

	/**
		* @brief Get data pointer.
		* Will dereference if it needs to.
		*/
	inline void* getData() const
	{
		if( Field_->isPointerType() )
		{
			return getRawData() != nullptr ? *reinterpret_cast< void** >( getRawData() ) : nullptr;
		}
		else
		{
			return getRawData();
		}
	}

	/**
		* @brief Check if it's a nullptr.
		*/
	inline bool isNullptr() const
	{
		if( Field_->isPointerType() )
		{
			auto Pointer = *reinterpret_cast< void** >( getRawData() );
			return Pointer == nullptr;
		}

		return false;
	}

	/**
		* @brief Get upper class.
		*/
	inline const ReClass* getUpperClass() const
	{
		const ReClass* FieldClass = static_cast< const ReClass* >( Field_->getType() );
		const ReClass* UpcastFieldType = FieldClass;
        if( FieldClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			UpcastFieldType = getData() != nullptr ? reinterpret_cast< ReObject* >( getData() )->getClass() : FieldClass;
		}
		return UpcastFieldType;
	}

	/**
		* @brief Get key upper class.
		* @pre Is a container.
		*/
	inline const ReClass* getKeyUpperClass() const
	{
		BcAssert( Field_->isContainer() );
		const ReClass* FieldClass = static_cast< const ReClass* >( Field_->getKeyType() );
		const ReClass* UpcastFieldType = FieldClass;
        if( FieldClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			UpcastFieldType = getData() != nullptr ? reinterpret_cast< ReObject* >( getData() )->getClass() : FieldClass;
		}
		return UpcastFieldType;
	}

	/**
		* @brief Get value upper class.
		* @pre Is a container.
		*/
	inline const ReClass* getValueUpperClass() const
	{
		BcAssert( Field_->isContainer() );
		const ReClass* FieldClass = static_cast< const ReClass* >( Field_->getValueType() );
		const ReClass* UpcastFieldType = FieldClass;
        if( FieldClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			UpcastFieldType = getData() != nullptr ? reinterpret_cast< ReObject* >( getData() )->getClass() : FieldClass;
		}
		return UpcastFieldType;
	}

	/**
		* @brief New write iterator. Owner must delete.
		* @pre Is a container.
		*/
	inline ReContainerAccessor::WriteIterator* newWriteIterator() const
	{
		BcAssert( Field_->isContainer() );
		return Field_->newWriteIterator( getRawData() );
	}

	/**
		* @brief New read iterator. Owner must delete.
		* @pre Is a container.
		*/
	inline ReContainerAccessor::ReadIterator* newReadIterator() const
	{
		BcAssert( Field_->isContainer() );
		return Field_->newReadIterator( getRawData() );
	}

	/**
		* @brief Is transient? I.e. ignore for serialisation.
		*/
	inline BcBool isTransient() const
	{
		return Field_->getFlags() & bcRFF_TRANSIENT;
	}
			
	/**
		* @brief Is to be a shallow copy?
		*/
	inline BcBool isShallowCopy() const
	{
		return Field_->getFlags() & bcRFF_SHALLOW_COPY;
	}

	/**
		* @brief Is field a pointer type?
	 	*/
	inline BcBool isPointerType() const
	{
		return Field_->isPointerType();
	}

	/**
		* @brief Is field a object type?
	 	*/
	inline BcBool isObjectType() const
	{
		const ReClass* FieldClass = static_cast< const ReClass* >( Field_->getType() );
        return FieldClass->hasBaseClass( ReObject::StaticGetClass() );
	}


	/**
		* @brief Is field a container type?
	 	*/
	inline BcBool isContainerType() const
	{
		return Field_->isContainer();
	}

	/**
		* @brief Is field a container of pointer keys?
	 	*/
	inline BcBool isContainerOfPointerKeys() const
	{
		return isContainerType() && ( Field_->getKeyFlags() & bcRFF_ANY_POINTER_TYPE );
	}

	/**
		* @brief Is field a container of pointer values?
	 	*/
	inline BcBool isContainerOfPointerValues() const
	{
		return isContainerType() && ( Field_->getValueFlags() & bcRFF_ANY_POINTER_TYPE );
	}

private:
	void* Data_;
	const ReField* Field_;

};

#endif // __REFLECTION_FIELDACCESSOR_H__
