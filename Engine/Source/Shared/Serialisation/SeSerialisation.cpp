#include "Serialisation/SeSerialisation.h"

//////////////////////////////////////////////////////////////////////////
// Statics
const char* SeISerialiser::SerialiserVersionString = "$SerialiserVersion";
const char* SeISerialiser::RootIDString = "$RootID";
const char* SeISerialiser::ObjectsString = "$Objects";
const char* SeISerialiser::ClassString = "$Class";
const char* SeISerialiser::IDString = "$ID";
const char* SeISerialiser::FieldString = "$Field";
const char* SeISerialiser::ValueString = "$Value";

//////////////////////////////////////////////////////////////////////////
// Unit tests.
#if !PSY_PRODUCTION
#include <catch.hpp>

#include "Reflection/ReReflection.h"

#include "Serialisation/SeSerialisation.h"
#include "Serialisation/SeJsonReader.h"
#include "Serialisation/SeJsonWriter.h"

#include "Base/BcString.h"

extern int SeUnitTest_init = 0;

namespace 
{
	class ObjectCodecBasic:
		public SeISerialiserObjectCodec
	{
	public:
		ObjectCodecBasic()
		{
		}

		BcBool shouldSerialiseContents( void* InData, const ReClass* InType ) override
		{
			return BcTrue;
		}

		std::string serialiseAsStringRef( void* InData, const ReClass* InType )
		{
			return "";
		}
	
		BcBool isMatchingField( const class ReField* Field, const std::string& Name )
		{
			std::string FieldName = *Field->getName();
			if( *Field->getName() == Name )
			{
				return BcTrue;
			}
			return BcFalse;
		}
	
		BcBool shouldSerialiseField( void* InData, BcU32 ParentFlags, const class ReField* Field ) override
		{
			return BcTrue;
		}

		BcBool findObject( void*& OutObject, const ReClass* Type, BcU32 Key ) override
		{
			OutObject = nullptr;
			return BcFalse;
		}

		BcBool findObject( void*& OutObject, const ReClass* Type, const std::string& Key ) override
		{
			OutObject = nullptr;
			return BcFalse;
		}
	};


	class JsonWriterTestBase
	{
	public:
		REFLECTION_DECLARE_BASE_NOAUTOREG( JsonWriterTestBase );

		JsonWriterTestBase()
		{
		}

		int A_ = 0;
		int B_ = 1;
		int C_ = 2;
		int D_ = 3;
		float E_ = 0.0f; 
		float F_ = 2.0f; 
		float G_ = 4.0f; 
		float H_ = 6.0f; 
	};

	REFLECTION_DEFINE_BASE( JsonWriterTestBase );

	void JsonWriterTestBase::StaticRegisterClass()
	{
		ReField* Fields[] = 
		{
			new ReField( "A_", &JsonWriterTestBase::A_, bcRFF_IMPORTER ),
			new ReField( "B_", &JsonWriterTestBase::B_, bcRFF_IMPORTER ),
			new ReField( "C_", &JsonWriterTestBase::C_, bcRFF_IMPORTER ),
			new ReField( "D_", &JsonWriterTestBase::D_, bcRFF_IMPORTER ),
			new ReField( "E_", &JsonWriterTestBase::E_, bcRFF_IMPORTER ),
			new ReField( "F_", &JsonWriterTestBase::F_, bcRFF_IMPORTER ),
			new ReField( "G_", &JsonWriterTestBase::G_, bcRFF_IMPORTER ),
			new ReField( "H_", &JsonWriterTestBase::H_, bcRFF_IMPORTER ),
		};
	
		ReRegisterClass< JsonWriterTestBase >( Fields );
	}
}


TEST_CASE( "Serialisation-JsonWriter-Base" )
{
	// Register.
	JsonWriterTestBase::StaticRegisterClass();

	// Create object to save.
	JsonWriterTestBase JsonWriterTestBase_;
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBase_, JsonWriterTestBase_.getClass() );

	const auto& Value = Writer.getValue();

	// Basic checks.
	REQUIRE( Value.type() == Json::objectValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].type() == Json::intValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].asUInt() == SERIALISER_VERSION );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].type() == Json::arrayValue );
	REQUIRE( Value[ SeISerialiser::RootIDString ].type() == Json::stringValue );

	// Object check.
	const auto& Object = Value[ SeISerialiser::ObjectsString ][ 0 ];
	REQUIRE( Object.type() == Json::objectValue );

	// Check members.
	REQUIRE( (int)BcStrAtoi( Object[ "A_" ].asCString() ) == JsonWriterTestBase_.A_ );
	REQUIRE( (int)BcStrAtoi( Object[ "B_" ].asCString() ) == JsonWriterTestBase_.B_ );
	REQUIRE( (int)BcStrAtoi( Object[ "C_" ].asCString() ) == JsonWriterTestBase_.C_ );
	REQUIRE( (int)BcStrAtoi( Object[ "D_" ].asCString() ) == JsonWriterTestBase_.D_ );
	REQUIRE( BcStrAtof( Object[ "E_" ].asCString() ) == JsonWriterTestBase_.E_ );
	REQUIRE( BcStrAtof( Object[ "F_" ].asCString() ) == JsonWriterTestBase_.F_ );
	REQUIRE( BcStrAtof( Object[ "G_" ].asCString() ) == JsonWriterTestBase_.G_ );
	REQUIRE( BcStrAtof( Object[ "H_" ].asCString() ) == JsonWriterTestBase_.H_ );
}


#endif // !PSY_PRODUCTION
