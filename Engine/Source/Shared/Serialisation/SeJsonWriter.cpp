#include "Serialisation/SeJsonWriter.h"

#include "Base/BcHash.h"

#include <fstream>

//////////////////////////////////////////////////////////////////////////
// Statics
const char* SeJsonWriter::SerialiserVersionEntry = "SerialiserVersion";
const char* SeJsonWriter::RootIDEntry = "RootID";
const char* SeJsonWriter::ObjectsEntry = "Objects";
const char* SeJsonWriter::ClassEntry = "Class";
const char* SeJsonWriter::IDEntry = "ID";
const char* SeJsonWriter::MembersEntry = "Members";
const char* SeJsonWriter::FieldEntry = "Field";
const char* SeJsonWriter::ValueEntry = "Value";

//////////////////////////////////////////////////////////////////////////
// Ctor
SeJsonWriter::SeJsonWriter( const char* FileName ):
	OutputFile_( FileName )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SeJsonWriter::~SeJsonWriter()
{

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
			auto ClassValue = serialiseClass( ClassToSerialise.pData_, static_cast< const ReClass* >( ClassToSerialise.pType_ ), true );

			ObjectsValue_.append( ClassValue );
		}
		else
		{
			// We should only be serialising classes here.
			BcAssert( false );
		}
	}

	// Write out root object.
	RootValue_ = Json::Value( Json::objectValue );
	RootValue_[ SerialiserVersionEntry ] = SERIALISER_VERSION;
	RootValue_[ RootIDEntry ] = ObjectsValue_[ Json::Value::UInt( 0 ) ][ IDEntry ];
	RootValue_[ ObjectsEntry ] = ObjectsValue_;

	//* test code.
	Json::StyledWriter Writer;
	std::ofstream OutStream;
	OutStream.open( OutputFile_ );
	OutStream << Writer.write( RootValue_ );
	OutStream.close();
	//*/

    return pData;
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
Json::Value SeJsonWriter::serialiseClass( void* pData, const ReClass* pClass, bool StoreID )
{
	// Don't bother will null classes.
	if( pData == nullptr )
	{
		return Json::Value( Json::nullValue );
	}

	// Setup Json::Value for this class.
	Json::Value ClassValue( Json::objectValue );
	ClassValue[ ClassEntry ] = *pClass->getName();
	if( StoreID )
	{
		ClassValue[ IDEntry ] = (BcU32)BcHash( pData );
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
				ClassValue[ ValueEntry ] = OutString;
			}
			else
			{
				ClassValue = OutString;
			}
		}
		else
		{
			// Members value.
			Json::Value MembersValue( Json::objectValue );

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
	
						MembersValue[ *pField->getName() ] = serialiseField( pData, pField );
					}
				}
				pProcessingClass = pProcessingClass->getSuper();
			}

			// Setup members.
			ClassValue[ MembersEntry ] = MembersValue;
		}
	}

	return ClassValue;
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
Json::Value SeJsonWriter::serialiseField( void* pData, const ReField* pField )
{
	// Select the appropriate serialise method to use if we
	// have some data to serialise.
	if( pData != nullptr )
	{
		if( pField->isContainer() )
		{
			if( pField->getKeyType() == nullptr )
			{
				return serialiseArray( pData, pField );
			}
			else
			{
				return serialiseDict( pData, pField );
			}
		}
		else
		{
			if( pField->getType()->isTypeOf< ReClass >() )
			{
				if( pField->isPointerType() == false )
				{
					return serialiseClass( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), false );
				}
				else
				{
					return serialisePointer( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ) );
				}
			}
		}
	}
	return Json::Value( Json::nullValue );
}

//////////////////////////////////////////////////////////////////////////
// serialisePointer
//virtual
Json::Value SeJsonWriter::serialisePointer( void* pData, const ReClass* pClass )
{
	if( pData != nullptr )
	{
		// Setup Json::Value for this class.
		Json::Value PointerValue = BcHash( pData );
	
		// Check if we can up cast.
		if( pClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			pClass = reinterpret_cast< ReObject* >( pData )->getClass();
		}
			
		// Add to list to serialise if it hasn't been added.
		auto ClassToSerialise = SerialiseClass( pData, pClass );
		if( std::find( SerialiseClasses_.begin(), SerialiseClasses_.end(), ClassToSerialise ) == SerialiseClasses_.end() )
		{
			SerialiseClasses_.push_back( ClassToSerialise );
		}

		return PointerValue;
	}

	return Json::nullValue;
}

//////////////////////////////////////////////////////////////////////////
// serialiseArray
//virtual
Json::Value SeJsonWriter::serialiseArray( void* pData, const ReField* pField )
{
	Json::Value ArrayValue( Json::arrayValue );
	auto pFieldValueType = pField->getValueType();
	auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
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
				ClassValue = serialiseClass( pValueData, static_cast< const ReClass* >( pFieldValueType ), false ); 
			}
			else
			{
				void* pPointerValueData = *reinterpret_cast< void** >( pValueData );
				ClassValue = serialisePointer( pPointerValueData, static_cast< const ReClass* >( pFieldValueType ) );
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
Json::Value SeJsonWriter::serialiseDict( void* pData, const ReField* pField )
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
					ClassValue = serialiseClass( pValueData, static_cast< const ReClass* >( pFieldValueType ), true ); // TODO: Only if pointer type.
				}
				else
				{
					void* pPointerValueData = *reinterpret_cast< void** >( pValueData );
					ClassValue = serialisePointer( pPointerValueData, static_cast< const ReClass* >( pFieldValueType ) );
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
