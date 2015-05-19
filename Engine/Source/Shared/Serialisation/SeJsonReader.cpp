#include "Serialisation/SeJsonReader.h"

#include "Base/BcMemory.h"

#include <fstream>
#include <algorithm>

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
void* SeJsonReader::internalSerialise( void* pData, const ReClass* pType )
{
	const Json::Value& RootIDEntry( RootValue_[ RootIDString ] );
	const Json::Value& VersionEntry( RootValue_[ SerialiserVersionString ] );
	const Json::Value& ObjectsValue( RootValue_[ ObjectsString ] );

	// Grab the file version.
	FileVersion_ = VersionEntry.asUInt();

	// Create all object types and add to list.
	for( auto It( ObjectsValue.begin() ); It != ObjectsValue.end(); ++It )
	{
		auto ObjectToSerialise( *It );
		auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassString ].asString() ) );
		if( ClassType->getTypeSerialiser() != nullptr )
		{
			std::string ID( ObjectToSerialise[ IDString ].asCString() );
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
		auto ClassType( ReManager::GetClass( ObjectToSerialise[ ClassString ].asString() ) );
		std::string ID( ObjectToSerialise[ IDString ].asCString() );
		auto ClassToSerialise( getSerialiseClass( ID, ClassType ) );

		// Add class to list for look up.
		serialiseClass( ClassToSerialise.pData_, ClassType, ObjectToSerialise, 0 );
	}

	return pData;
}

//////////////////////////////////////////////////////////////////////////
// internalSerialiseString
std::string SeJsonReader::internalSerialiseString( void* pData, const ReClass* pType )
{
	BcBreakpoint;
	return "";
}

//////////////////////////////////////////////////////////////////////////
// serialiseClass
//virtual
void SeJsonReader::serialiseClass( void* pData, const ReClass* pClass, const Json::Value& InputValue, BcU32 ParentFlags )
{
	// Get type serialiser.
	auto Serialiser = pClass->getTypeSerialiser();
	std::string OutString;

	BcBool Success = false;
	// Attempt conversion to string.
	if( Serialiser != nullptr )
	{	
		if( InputValue.type() == Json::stringValue &&
			Serialiser->serialiseFromString( pData, InputValue.asString() ) )
		{
			Success = true;
		}
		// Attempt conversion to float via string.
		else if( InputValue.type() == Json::realValue )
		{
			Success = Serialiser->serialiseFromString( pData, std::to_string( InputValue.asDouble() ) );
		}
		// Attempt conversion to uint via string.
		else if( InputValue.type() == Json::uintValue )
		{
			Success = Serialiser->serialiseFromString( pData, std::to_string( InputValue.asUInt() ) );
		}
		// Attempt conversion to int via string.
		else if( InputValue.type() == Json::intValue )
		{
			Success = Serialiser->serialiseFromString( pData, std::to_string( InputValue.asInt() ) );
		}
		// Attempt conversion to bool via string.
		else if( InputValue.type() == Json::booleanValue )
		{
			Success = Serialiser->serialiseFromString( pData, std::to_string( InputValue.asBool() ) );
		}
		// Attempt conversion to object.
		else if( InputValue.type() == Json::objectValue )
		{
			Json::Value ValueValue = InputValue.get( ValueString, Json::nullValue );
			if( ValueValue.type() == Json::stringValue &&
				Serialiser->serialiseFromString( pData, ValueValue.asString() ) )
			{
				Success = true;
			}
		}
		else
		{
			PSY_LOG( "ERROR: Unable to serialise type \"%s\"\n", (*pClass->getName()).c_str() );
			return;
		}
	}

	if( Success == false )
	{
		// Attempt to read in as class members.
		if( InputValue.type() == Json::objectValue )
		{
			// If it is a binary data object, we can serialise any POD type into it.
			static const ReClass* BinaryDataClass = ReManager::GetClass( "BcBinaryData" );
			if( pClass == BinaryDataClass )
			{
				auto ClassToSerialise = getSerialiseClass( InputValue, nullptr );
				BcAssert( ClassToSerialise.pType_->getFlags() & bcRFF_POD );
				if( ClassToSerialise.pType_->getFlags() & bcRFF_POD )
				{
					serialiseClass( ClassToSerialise.pData_, ClassToSerialise.pType_, InputValue, ParentFlags );

					// Pass into binary data.
					BcBinaryData* BinaryData = static_cast< BcBinaryData* >( pData );
					*BinaryData = BcBinaryData( ClassToSerialise.pData_, ClassToSerialise.pType_->getSize(), BcTrue );

					// Free original data now that it's copied in.
					ClassToSerialise.pType_->destroy( ClassToSerialise.pData_ );
				}
			}
			else
			{
				serialiseClassMembers( pData, pClass, InputValue, ParentFlags );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseClassMembers
//virtual
void SeJsonReader::serialiseClassMembers( void* pData, const ReClass* pClass, const Json::Value& MemberValues, BcU32 ParentFlags )
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
					pData, ParentFlags, pField ) )
				{
					if( FoundMember != Members.end() )
					{
						serialiseField( pData, pField, MemberValues[ *FoundMember ], ParentFlags );
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
void SeJsonReader::serialiseField( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags )
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
				serialiseArray( pData, pField, InputValue, ParentFlags );
			}
			else
			{
				serialiseDict( pData, pField, InputValue, ParentFlags );
			}
		}
		else
		{
			if( pField->isPointerType() == false )
			{
				serialiseClass( pField->getData< void >( pData ), static_cast< const ReClass* >( pField->getType() ), InputValue, ParentFlags );
			}
			else
			{
				void* pFieldData = nullptr;
				serialisePointer( pFieldData, static_cast< const ReClass* >( pField->getType() ), pField->getFlags(), InputValue, ParentFlags, true );
				void** pOutputFieldData = reinterpret_cast< void** >( reinterpret_cast< BcU8* >( pData ) + pField->getOffset() );
				*pOutputFieldData = pFieldData;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// serialisePointer
//virtual
void SeJsonReader::serialisePointer( void*& pData, const ReClass* pClass, BcU32 FieldFlags, const Json::Value& InputValue, BcU32 ParentFlags, BcBool IncrementRefCount )
{
	SeJsonReader::SerialiseClass ClassToSerialise( "", nullptr, nullptr );

	// Check input type.
	if( InputValue.type() == Json::intValue )
	{
		ClassToSerialise = getSerialiseClass( InputValue.asUInt(), pClass );
	}
	else if( InputValue.type() == Json::stringValue )
	{
		ClassToSerialise = getSerialiseClass( InputValue.asString(), pClass );
	}
	else if( InputValue.type() == Json::objectValue )
	{
		ClassToSerialise = getSerialiseClass( InputValue, pClass );
		auto ClassType( ClassToSerialise.pType_ );
		serialiseClass( ClassToSerialise.pData_, ClassType, InputValue, ParentFlags );
	}

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
void SeJsonReader::serialiseArray( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags )
{
	Json::Value ArrayValue( Json::arrayValue );
	auto FieldValueClass = pField->getValueType();
	auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

	// Clear container out.
	pWriteIterator->clear();

	// Iterate over Json values.
	if( InputValue.type() == Json::arrayValue )
	{
		// Handle json array.
		for( auto ValueIt( InputValue.begin() ); ValueIt != InputValue.end(); ++ValueIt )
		{
			if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
			{
				void* pTemporaryValue = FieldValueClass->create< void >();
				serialiseClass( pTemporaryValue, FieldValueClass, (*ValueIt), ParentFlags );
				pWriteIterator->add( pTemporaryValue );
				FieldValueClass->destroy( pTemporaryValue );
			}
			else
			{
				void* pTemporaryPointer = nullptr;
				serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), (*ValueIt), ParentFlags, false );
				pWriteIterator->add( &pTemporaryPointer );
			}
		}
	}
	else
	{
		// Treat as single value.
		if( ( pField->getValueFlags() & bcRFF_SIMPLE_DEREF ) == 0 )
		{
			void* pTemporaryValue = FieldValueClass->create< void >();
			serialiseClass( pTemporaryValue, FieldValueClass, InputValue, ParentFlags );
			pWriteIterator->add( pTemporaryValue );
			FieldValueClass->destroy( pTemporaryValue );
		}
		else
		{
			void* pTemporaryPointer = nullptr;
			serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), InputValue, ParentFlags, false );
			pWriteIterator->add( &pTemporaryPointer );
		}
	}

	delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// serialiseDict
//virtual
void SeJsonReader::serialiseDict( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags )
{
	Json::Value ArrayValue( Json::arrayValue );
	auto FieldKeyClass = pField->getKeyType();
	auto FieldValueClass = pField->getValueType();
	auto KeySerialiser = FieldKeyClass->getTypeSerialiser();
	auto pWriteIterator = pField->newWriteIterator( pField->getData< void >( pData ) );

	// Clear container out.
	pWriteIterator->clear();

	// Construct a temporary value & key.
	void* pTemporaryKey = FieldKeyClass->create< void >();

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
				void* pTemporaryValue = FieldValueClass->create< void >();
				serialiseClass( pTemporaryValue, FieldValueClass, Value, ParentFlags );
				pWriteIterator->add( pTemporaryKey, pTemporaryValue );
				FieldValueClass->destroy( pTemporaryValue );
			}
			else
			{
				void* pTemporaryPointer = nullptr;
				serialisePointer( pTemporaryPointer, FieldValueClass, pField->getValueFlags(), Value, ParentFlags, false );
				pWriteIterator->add( pTemporaryKey, &pTemporaryPointer );
			}
		}
	}

	FieldKeyClass->destroy( pTemporaryKey );

	delete pWriteIterator;
}

//////////////////////////////////////////////////////////////////////////
// getSerialiseClass
SeJsonReader::SerialiseClass SeJsonReader::getSerialiseClass( BcU32 ID, const ReClass* pType )
{
	SerialiseClass RetVal( "", nullptr, nullptr );

	if( ObjectCodec_->findObject( RetVal.pData_, pType, ID ) )
	{
		RetVal.pType_ = pType;
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// getSerialiseClass
SeJsonReader::SerialiseClass SeJsonReader::getSerialiseClass( std::string ID, const ReClass* pType )
{
	SerialiseClass RetVal( "", nullptr, nullptr );

	if( ObjectCodec_->findObject( RetVal.pData_, pType, ID ) )
	{
		RetVal.pType_ = pType;
	}
	else
	{
		auto FoundClass = std::find( SerialiseClasses_.begin(), SerialiseClasses_.end(), SerialiseClass( ID, nullptr, pType ) );
		if( FoundClass != SerialiseClasses_.end() )
		{
			RetVal = *FoundClass;
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// getSerialiseClass
SeJsonReader::SerialiseClass SeJsonReader::getSerialiseClass( const Json::Value& Value, const ReClass* pType )
{
	auto ClassType( ReManager::GetClass( Value[ ClassString ].asString() ) );
	SeJsonReader::SerialiseClass ClassToSerialise( "", nullptr, ClassType );
	ClassToSerialise.pData_ = ClassType->create< void >();
	return ClassToSerialise;
}