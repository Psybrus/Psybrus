#include "Serialisation/SeJsonReader.h"

#include "Base/BcMemory.h"

#include <fstream>

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////
// Statics
const char* SeJsonReader::SerialiserVersionEntry = "$SerialiserVersion";
const char* SeJsonReader::RootIDEntry = "$RootID";
const char* SeJsonReader::ObjectsEntry = "$Objects";
const char* SeJsonReader::ClassEntry = "$Class";
const char* SeJsonReader::IDEntry = "$ID";
const char* SeJsonReader::FieldEntry = "$Field";
const char* SeJsonReader::ValueEntry = "$Value";

//////////////////////////////////////////////////////////////////////////
// Ctor
SeJsonReader::SeJsonReader( 
		SeISerialiserObjectCodec* ObjectCodec ) :
	ObjectCodec_( ObjectCodec )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SeJsonReader::~SeJsonReader()
{

}

//////////////////////////////////////////////////////////////////////////
// load
void SeJsonReader::load( std::string FileName )
{
    // Read in the json file.
    Json::Reader Reader;
    std::ifstream InStream;
    InStream.open( FileName );
    Reader.parse( InStream, RootValue_ );
    InStream.close();
}

//////////////////////////////////////////////////////////////////////////
// getVersion
//virtual
BcU32 SeJsonReader::getVersion() const
{
    return SERIALISER_VERSION;
}

//////////////////////////////////////////////////////////////////////////
// getFileVersion
//virtual
BcU32 SeJsonReader::getFileVersion() const
{
    return FileVersion_;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialise
//virtual
void* SeJsonReader::internalSerialise( void* pData, const ReType* pType )
{
	const Json::Value& RootIDEntry( RootValue_[ RootIDEntry ] );
	const Json::Value& VersionEntry( RootValue_[ SerialiserVersionEntry ] );
	const Json::Value& ObjectsValue( RootValue_[ ObjectsEntry ] );

	// Grab the file version.
	FileVersion_ = VersionEntry.asUInt();

	// Create all object types and add to list.
	for( auto It( ObjectsValue.begin() ); It != ObjectsValue.end(); ++It )
	{
		auto ObjectToSerialise( *It );
		auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassEntry ].asString() ) );
		if( ClassType->getTypeSerialiser() != nullptr )
		{
			std::string ID( ObjectToSerialise[ IDEntry ].asCString() );
			void* pClassObject = nullptr;
			if( ID == RootIDEntry.asCString() )
			{
				if( pData != nullptr )
				{
					pClassObject = pData;
				}
				else
				{
					pClassObject = ClassType->create< void >();
					pData = pClassObject;
				}
			}
			else
			{
				pClassObject = ClassType->create< void >();
			}

			// Add class to list for look up.
			SerialiseClasses_.push_back( SerialiseClass( ID, pClassObject, ClassType ) );
		}
	}

	// Serialise in.
	for( auto It( ObjectsValue.begin() ); It != ObjectsValue.end(); ++It )
	{
		auto ObjectToSerialise( *It );
		auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassEntry ].asString() ) );
		std::string ID( ObjectToSerialise[ IDEntry ].asCString() );
		auto ClassToSerialise( getSerialiseClass( ID, ClassType ) );

		// Add class to list for look up.
		serialiseClass( ClassToSerialise.pData_, ClassType, ObjectToSerialise );
	}

    return pData;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialiseString
std::string SeJsonReader::internalSerialiseString( void* pData, const ReType* pType )
{
	BcBreakpoint;
	return "";
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
void SeJsonReader::serialiseClass( void* pData, const ReClass* pClass, const Json::Value& InputValue )
{
    // Get type serialiser.
    auto Serialiser = pClass->getTypeSerialiser();
    std::string OutString;

    bool Success = false;

    // Attempt conversion to string.
	if( Serialiser != nullptr )
	{
		if( InputValue.type() == Json::stringValue &&
			Serialiser->serialiseFromString( pData, InputValue.asString() ) )
		{
			Success = true;
		}
		// Attempt conversion to float via string.
		else if( InputValue.type() == Json::realValue &&
			Serialiser->serialiseFromString( pData, boost::lexical_cast< std::string >( InputValue.asDouble() ) ) )
		{
			Success = true;
		}
		// Attempt conversion to uint via string.
		else if( InputValue.type() == Json::uintValue &&
			Serialiser->serialiseFromString( pData, boost::lexical_cast< std::string >( InputValue.asUInt() ) ) )
		{
			Success = true;
		}
		// Attempt conversion to int via string.
		else if( InputValue.type() == Json::intValue &&
			Serialiser->serialiseFromString( pData, boost::lexical_cast< std::string >( InputValue.asInt() ) ) )
		{
			Success = true;
		}
		// Attempt conversion to bool via string.
		else if( InputValue.type() == Json::booleanValue &&
			Serialiser->serialiseFromString( pData, boost::lexical_cast< std::string >( InputValue.asBool() ) ) )
		{
			Success = true;
		}
		// Attempt conversion to object.
		else if( InputValue.type() == Json::objectValue )
		{
			Json::Value ValueValue = InputValue.get( ValueEntry, Json::nullValue );
			if( ValueValue.type() == Json::stringValue &&
				Serialiser->serialiseFromString( pData, ValueValue.asString() ) )
			{
				Success = true;
			}
		}
		else
		{
			BcPrintf( "ERROR: Unable to serialise type \"%s\"\n", (*pClass->getName()).c_str() );
			return;
		}
	}

    if( Success == false )
    {
		// Attempt to read in as class members.
		if( InputValue.type() == Json::objectValue )
		{
			serialiseClassMembers( pData, pClass, InputValue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseClassMembers
//virtual
void SeJsonReader::serialiseClassMembers( void* pData, const ReClass* pClass, const Json::Value& MemberValues )
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
				auto Members = MemberValues.getMemberNames();
				auto FoundMember = std::find_if( Members.begin(), Members.end(), 
					[ this, pField ]( const std::string& Member )
					{
						return ObjectCodec_->isMatchingField( pField, Member );
					} );
				if( ObjectCodec_->shouldSerialiseField( 
					pData, pField ) )
				{
					if( FoundMember != Members.end() )
					{
						serialiseField( pData, pField, MemberValues[ *FoundMember ] );
					}
				}
			}
		}
		pProcessingClass = pProcessingClass->getSuper();
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseField
//virtual
void SeJsonReader::serialiseField( void* pData, const ReField* pField, const Json::Value& InputValue )
{	
	// Select the appropriate serialise method to use if we
    // have some data to serialise.
    if( pData != nullptr )
    {
        if( pField->isContainer() )
        {
            if( pField->getKeyType() == nullptr )
            {
                serialiseArray( pData, pField, InputValue );
            }
            else
            {
                serialiseDict( pData, pField, InputValue );
            }
        }
        else
        {
            if( pField->getType()->isTypeOf< ReClass >() )
            {
                if( pField->isPointerType() == false )
                {
                    serialiseClass( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), InputValue );
                }
                else
                {
                    void* pFieldData = nullptr;
                    serialisePointer( pFieldData, static_cast< const ReClass* >( pField->getType() ), pField->getFlags(), InputValue, true );
                    void** pOutputFieldData = reinterpret_cast< void** >( reinterpret_cast< BcU8* >( pData ) + pField->getOffset() );
                    *pOutputFieldData = pFieldData;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// serialisePointer
//virtual
void SeJsonReader::serialisePointer( void*& pData, const ReClass* pClass, BcU32 FieldFlags, const Json::Value& InputValue, BcBool IncrementRefCount )
{
    auto ClassToSerialise = getSerialiseClass( InputValue.asCString(), pClass );
    if( ClassToSerialise.pData_ != nullptr && ClassToSerialise.pData_ != pData )
    {
        if( ( FieldFlags & bcRFF_SIMPLE_DEREF ) != 0 )
        {
            pData = ClassToSerialise.pData_;

#if REFLECTION_ENABLE_GC
            // Object references need ref count increased.
            if( ( FieldFlags & bcRFF_OBJECT_REFERENCE ) != 0 && IncrementRefCount )
            {
                reinterpret_cast< ReObject* >( pData )->incRefCount();
            }
#endif
        }
    }
    else
    {
        pData = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
// serialiseArray
//virtual
void SeJsonReader::serialiseArray( void* pData, const ReField* pField, const Json::Value& InputValue )
{
    Json::Value ArrayValue( Json::arrayValue );
    auto pFieldValueType = pField->getValueType();
    auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
    auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

    // Clear container out.
    pWriteIterator->clear();

    // Construct a temporary value.
 	BcAssert( pFieldValueType->isTypeOf< ReClass >() );
	const ReClass* FieldValueClass = static_cast< const ReClass* >( pFieldValueType );
	void* pTemporaryValue = FieldValueClass->create< void >();

    // Iterate over Json values.
	if( InputValue.type() == Json::arrayValue )
	{
		// Handle json array.
		for( auto ValueIt( InputValue.begin() ); ValueIt != InputValue.end(); ++ValueIt )
		{
			if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
			{
				serialiseClass( pTemporaryValue, FieldValueClass, (*ValueIt) );
				pWriteIterator->add( pTemporaryValue );
			}
			else
			{
				void* pTemporaryPointer = nullptr;
				serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), (*ValueIt), false );
				pWriteIterator->add( &pTemporaryPointer );
			}
		}
	}
	else
	{
		// Treat as single value.
		if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
		{
			serialiseClass( pTemporaryValue, FieldValueClass, InputValue );
			pWriteIterator->add( pTemporaryValue );
		}
		else
		{
			void* pTemporaryPointer = nullptr;
			serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), InputValue, false );
			pWriteIterator->add( &pTemporaryPointer );
		}
	}

	// Free temporary value.
    FieldValueClass->destroy( pTemporaryValue );

    delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// serialiseDict
//virtual
void SeJsonReader::serialiseDict( void* pData, const ReField* pField, const Json::Value& InputValue )
{
    Json::Value ArrayValue( Json::arrayValue );
    auto pFieldKeyType = pField->getKeyType();
    auto pFieldValueType = pField->getValueType();
    auto KeySerialiser = pFieldKeyType->getTypeSerialiser();
    auto ValueSerialiser = pFieldValueType->getTypeSerialiser();
    auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

    // Clear container out.
    pWriteIterator->clear();

    // Construct a temporary value & key.
	BcAssert( pFieldKeyType->isTypeOf< ReClass >() );
	BcAssert( pFieldValueType->isTypeOf< ReClass >() );
	const ReClass* FieldKeyClass = static_cast< const ReClass* >( pFieldKeyType );
	const ReClass* FieldValueClass = static_cast< const ReClass* >( pFieldValueType );
    void* pTemporaryKey = FieldKeyClass->create< void >();
    void* pTemporaryValue = FieldValueClass->create< void >();

    // Iterate over Json member values.
    auto MemberKeys = InputValue.getMemberNames();
    for( auto KeyIt( MemberKeys.begin() ); KeyIt != MemberKeys.end(); ++KeyIt )
    {
        auto Value = InputValue[ *KeyIt ];

        // Serialise key.
        if( KeySerialiser->serialiseFromString( pTemporaryKey, *KeyIt ) )
        {
            if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
            {
                // Serialise value.
                serialiseClass( pTemporaryValue, FieldValueClass, Value );
                pWriteIterator->add( pTemporaryKey, pTemporaryValue );
            }
            else
            {
                void* pTemporaryPointer = nullptr;
                serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), Value, false );
                pWriteIterator->add( pTemporaryKey, &pTemporaryPointer );
            }
        }
    }

    // Free temporary value.
	FieldKeyClass->destroy( pTemporaryKey );
    FieldValueClass->destroy( pTemporaryValue );

    delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// getSerialiseClass
SeJsonReader::SerialiseClass SeJsonReader::getSerialiseClass( std::string ID, const ReType* pType )
{
    auto FoundClass = std::find( SerialiseClasses_.begin(), SerialiseClasses_.end(), SerialiseClass( ID, nullptr, pType ) );
    if( FoundClass != SerialiseClasses_.end() )
    {
        return *FoundClass;
    }
    return SerialiseClass( 0, nullptr, nullptr );
}
