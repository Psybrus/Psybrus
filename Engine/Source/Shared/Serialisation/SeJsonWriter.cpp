#include "Serialisation/SeJsonWriter.h"

#include <fstream>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Ctor
SeJsonWriter::SeJsonWriter( 
		SeISerialiserObjectCodec* ObjectCodec ) :
	ObjectCodec_( ObjectCodec )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SeJsonWriter::~SeJsonWriter()
{

}

//////////////////////////////////////////////////////////////////////////
// save
void SeJsonWriter::save( std::string FileName )
{
	Json::StyledWriter Writer;
	std::ofstream OutStream;
	OutStream.open( FileName );
	OutStream << Output_;
	OutStream.close();
}

//////////////////////////////////////////////////////////////////////////
// getVersion
//virtual
BcU32 SeJsonWriter::getVersion() const
{
	return SERIALISER_VERSION;
}

//////////////////////////////////////////////////////////////////////////
// getFileVersion
//virtual
BcU32 SeJsonWriter::getFileVersion() const
{
	return SERIALISER_VERSION;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialise
//virtual
void* SeJsonWriter::internalSerialise( void* pData, const ReType* pType )
{
	Output_ = internalSerialiseString( pData, pType );

    return pData;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialiseString
//virtual
std::string SeJsonWriter::internalSerialiseString( void* pData, const ReType* pType )
{
	// Setup list to use for serialising.
	ObjectsValue_ = Json::Value( Json::arrayValue );

	// Push to list of classes to serialise.
	SerialiseClasses_.push_back( SerialiseClass( pData, pType ) );

	// Iterate over classes to serialise. This list will get larger as we go.
	for( auto It( SerialiseClasses_.begin() ); It != SerialiseClasses_.end(); ++It )
	{
		// Grab class to serialise.
		auto ClassToSerialise( (*It) );

		// Check we're a class so we know we can serialise.
		if( ClassToSerialise.pType_->isTypeOf< ReClass >() )
		{
			auto ID = ObjectCodec_->serialiseAsStringRef( ClassToSerialise.pData_, ClassToSerialise.pType_ );
			auto ClassValue = serialiseClass( ClassToSerialise.pData_, static_cast< const ReClass* >( ClassToSerialise.pType_ ), 0, true );
			ObjectValueMap_[ ID ] = ClassValue;
		}
		else
		{
			// We should only be serialising classes here.
			BcAssert( false );
		}
	}

	// Grab root from map and place in first.
	auto RootID = ObjectCodec_->serialiseAsStringRef( pData, pType );
	ObjectsValue_.append( ObjectValueMap_[ RootID ] );
	ObjectValueMap_.erase( ObjectValueMap_.find( RootID ) );

	// Add to value map.
	for( auto& Value : ObjectValueMap_ )
	{
		ObjectsValue_.append( Value.second );
	}

	// Write out root object.
	RootValue_ = Json::Value( Json::objectValue );
	RootValue_[ SerialiserVersionString ] = SERIALISER_VERSION;
    RootValue_[ RootIDString ] = ObjectsValue_[ Json::Value::UInt( 0 ) ][ IDString ];
	RootValue_[ ObjectsString ] = ObjectsValue_;

	//* test code.
	Json::StyledWriter Writer;
	std::string Output;
	Output = Writer.write( RootValue_ );

    return Output;
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
Json::Value SeJsonWriter::serialiseClass( void* pData, const ReClass* pClass, BcU32 ParentFlags, bool StoreID )
{
	// Don't bother will null classes.
	if( pData == nullptr )
	{
		return Json::Value( Json::nullValue );
	}

	// Setup Json::Value for this class.
	Json::Value ClassValue( Json::objectValue );
	ClassValue[ ClassString ] = *pClass->getName();
	if( StoreID )
	{
		ClassValue[ IDString ] = ObjectCodec_->serialiseAsStringRef( pData, pClass );
	}

	// If the object codec says we dont want, don't serialise its contents.
	// Only serialise it as an ID.
	if( !ObjectCodec_->shouldSerialiseContents( pData, pClass ) )
	{
		return ClassValue[ IDString ];
	}

	// Get type serialiser.
	auto Serialiser = pClass->getTypeSerialiser();

	if( Serialiser != nullptr )
	{
		std::string OutString;

		// Attempt conversion to string if we don't need to store the ID.
		if( Serialiser->serialiseToString( pData, OutString ) )
		{
			// Setup value.
			if( StoreID )
			{
				ClassValue[ ValueString ] = OutString;
			}
			else
			{
				ClassValue = OutString;
			}
		}
		else
		{
			// Iterate over members to add, all supers too.
			const ReClass* pProcessingClass = pClass;
			while( pProcessingClass != nullptr )
			{
				// If this class has fields, then iterate over them.
				if( pProcessingClass->getNoofFields() > 0 )
				{
					for( BcU32 Idx = 0; Idx < pProcessingClass->getNoofFields(); ++Idx )
					{
						const ReField* pField = pProcessingClass->getField( Idx );
	
						// Check if we should serialise this field.
						if ( ObjectCodec_->shouldSerialiseField( pData, ParentFlags, pField ) )
						{
							ClassValue[ *pField->getName() ] = serialiseField( pData, pField, ParentFlags );
						}
					}
				}
				pProcessingClass = pProcessingClass->getSuper();
			}
		}
	}

	return ClassValue;
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
Json::Value SeJsonWriter::serialiseField( void* pData, const ReField* pField, BcU32 ParentFlags )
{
	// Add our flags to the parent flags. These should propagate down the hierarchy.
	ParentFlags |= pField->getFlags();

	// Select the appropriate serialise method to use if we
	// have some data to serialise.
	if( pData != nullptr )
	{
		if( pField->isContainer() )
		{
			if( pField->getKeyType() == nullptr )
			{
				return serialiseArray( pData, pField, ParentFlags );
			}
			else
			{
				return serialiseDict( pData, pField, ParentFlags );
			}
		}
		else
		{
			if( pField->getType()->isTypeOf< ReClass >() )
			{
				if( pField->isPointerType() == false )
				{
					return serialiseClass( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), ParentFlags, false );
				}
				else
				{
					return serialisePointer( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), ParentFlags );
				}
			}
		}
	}
	return Json::Value( Json::nullValue );
}

//////////////////////////////////////////////////////////////////////////
// serialisePointer
//virtual
Json::Value SeJsonWriter::serialisePointer( void* pData, const ReClass* pClass, BcU32 ParentFlags )
{
	if( pData != nullptr )
	{
		// Setup Json::Value for this class.
		Json::Value PointerValue = ObjectCodec_->serialiseAsStringRef( pData, pClass );
	
		// Check if we can up cast.
		if( pClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			ReObject* Object = reinterpret_cast< ReObject* >( pData );
			pClass = Object->getClass();
		}
			
		// Add to list to serialise if it hasn't been added.
		if( ObjectCodec_->shouldSerialiseContents( pData, pClass ) )
		{
			auto ClassToSerialise = SerialiseClass( pData, pClass );
			if( std::find( SerialiseClasses_.begin(), SerialiseClasses_.end(), ClassToSerialise ) == SerialiseClasses_.end() )
			{
				SerialiseClasses_.push_back( ClassToSerialise );
			}
		}

		return PointerValue;
	}

	return Json::nullValue;
}

//////////////////////////////////////////////////////////////////////////
// serialiseArray
//virtual
Json::Value SeJsonWriter::serialiseArray( void* pData, const ReField* pField, BcU32 ParentFlags )
{
	Json::Value ArrayValue( Json::arrayValue );
	auto pFieldValueType = pField->getValueType();
	auto pReadIterator = pField->newReadIterator( pField->getData< void >( pData ) );

	// Iterate over values and serialise individually.
	while( pReadIterator->isValid() )
	{
		void* pValueData = pReadIterator->getValue();

		// Only serialise class types.
		if( pFieldValueType->isTypeOf< ReClass >() )
		{
			Json::Value ClassValue;
			if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
			{
				ClassValue = serialiseClass( pValueData, static_cast< const ReClass* >( pFieldValueType ), ParentFlags, false ); 
			}
			else
			{
				void* pPointerValueData = *reinterpret_cast< void** >( pValueData );
				ClassValue = serialisePointer( pPointerValueData, static_cast< const ReClass* >( pFieldValueType ), ParentFlags );
			}

			ArrayValue.append( ClassValue );
		}
		else
		{
			BcAssert( false );
		}

		pReadIterator->next();
	}

	delete pReadIterator;

	return ArrayValue;
}

//////////////////////////////////////////////////////////////////////////
// serialiseDict
//virtual
Json::Value SeJsonWriter::serialiseDict( void* pData, const ReField* pField, BcU32 ParentFlags )
{
	Json::Value DictValue( Json::objectValue );
	auto pFieldValueType = pField->getValueType();
	auto pFieldKeyType = pField->getKeyType();
	auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
	auto KeySerialiser = pFieldKeyType->getTypeSerialiser();
	auto pReadIterator = pField->newReadIterator( pField->getData< void >( pData ) );

	// Run a check to make sure we are not a simple deref field.
	if( ( pField->getKeyFlags() & bcRFF_SIMPLE_DEREF ) != 0 )
	{
		BcAssert( false );
		return Json::nullValue;
	}

	// Early out if we can't serialise.
	if( KeySerialiser == nullptr )
	{
		BcPrintf( "SeJsonWriter: Unable to serialise for key \"%s\"\n", ( *pFieldKeyType->getName() ).c_str() );
		return Json::nullValue;
	}

	if( ValueSerialiser == nullptr )
	{
		BcPrintf( "SeJsonWriter: Unable to serialise for value \"%s\"\n", ( *pFieldValueType->getName() ).c_str() );
		return Json::nullValue;
	}

	std::string OutKeyString;

	// Iterate over values and serialise individually.
	// NOTE: Json only supports strings as keys, therefore
	//       if serialising to Json, we must only support strings too.
	//       We could support any object, but it's unlikely we will
	//       use anything that isn't serialisable to strings.
	while( pReadIterator->isValid() )
	{
		void* pValueData = pReadIterator->getValue();
		void* pKeyData = pReadIterator->getKey();

		if( KeySerialiser->serialiseToString( pKeyData, OutKeyString ) )
		{
			// TODO: Implement enum types also.
			if( pFieldValueType->isTypeOf< ReClass >() )
			{
				Json::Value ClassValue;
				if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
				{
					ClassValue = serialiseClass( pValueData, static_cast< const ReClass* >( pFieldValueType ), ParentFlags, true ); // TODO: Only if pointer type.
				}
				else
				{
					void* pPointerValueData = *reinterpret_cast< void** >( pValueData );
					ClassValue = serialisePointer( pPointerValueData, static_cast< const ReClass* >( pFieldValueType ), ParentFlags );
				}
				DictValue[ OutKeyString ] = ClassValue;
			}
			else
			{
				BcAssert( false ); // This should never be hit.
			}
		}
		else
		{
			BcAssert( false ); // This should never be hit. It means we're using an invalid key type.
		}

		pReadIterator->next();
	}

	delete pReadIterator;

	return DictValue;
}	
