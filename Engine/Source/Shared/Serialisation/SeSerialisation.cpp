#include "Serialisation/SeSerialisation.h"

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

namespace TestData
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

		std::string serialiseAsStringRef( void* InData, const ReClass* InType ) override
		{
			BcU32 ID = ID_;
			if( IDMap_.find( InData ) == IDMap_.end() )
			{
				ID_++;
				IDMap_[ InData ] = ID_;
				ID = ID_;
			}
			else
			{
				ID = IDMap_[ InData ];
			}

			std::array< char, 128 > Buffer;
			BcSPrintf( Buffer.data(), Buffer.size(), "%u", ID );
			return Buffer.data();
		}
	
		BcBool isMatchingField( const class ReField* Field, const std::string& Name ) override
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

		int ID_ = 0;
		std::map< void*, int > IDMap_;
	};


	class TestBase
	{
	public:
		REFLECTION_DECLARE_BASE_NOAUTOREG( TestBase );

		TestBase()
		{
		}

		TestBase( bool Alternative )
		{
			if( Alternative )
			{
				A_ = 3;
				B_ = 2;
				C_ = 1;
				D_ = 0;
				E_ = 2.0f; 
				F_ = 0.0f; 
				G_ = "AltTestG"; 
				H_ = "AltTestH"; 
			}
		}

		bool operator == ( const TestBase& Other ) const
		{
			return A_ == Other.A_ && B_ == Other.B_ &&
				C_ == Other.C_ && D_ == Other.D_ &&
				E_ == Other.E_ && F_ == Other.F_ &&
				G_ == Other.G_ && H_ == Other.H_;
		}

		void test( const Json::Value& Object ) const
		{
			REQUIRE( (BcU8)BcStrAtoi( Object[ "A_" ].asCString() ) == A_ );
			REQUIRE( (BcS8)BcStrAtoi( Object[ "B_" ].asCString() ) == B_ );
			REQUIRE( (BcU32)BcStrAtoi( Object[ "C_" ].asCString() ) == C_ );
			REQUIRE( (BcS32)BcStrAtoi( Object[ "D_" ].asCString() ) == D_ );
			REQUIRE( BcStrAtof( Object[ "E_" ].asCString() ) == E_ );
			REQUIRE( BcStrAtof( Object[ "F_" ].asCString() ) == F_ );
			REQUIRE( G_ == Object[ "G_" ].asCString() );
			REQUIRE( H_ == Object[ "H_" ].asCString() );
		}

		BcU8 A_ = 1;
		BcS8 B_ = -1;
		BcU32 C_ = 2;
		BcS32 D_ = -2;
		BcF32 E_ = 0.0f; 
		BcF64 F_ = 2.0f; 
		std::string G_ = "TestG"; 
		std::string H_ = "TestH"; 
	};

	REFLECTION_DEFINE_BASE( TestBase );

	void TestBase::StaticRegisterClass()
	{
		static bool IsRegistered = false;
		if( IsRegistered == false )
		{
			ReField* Fields[] = 
			{
				new ReField( "A_", &TestBase::A_, bcRFF_IMPORTER ),
				new ReField( "B_", &TestBase::B_, bcRFF_IMPORTER ),
				new ReField( "C_", &TestBase::C_, bcRFF_IMPORTER ),
				new ReField( "D_", &TestBase::D_, bcRFF_IMPORTER ),
				new ReField( "E_", &TestBase::E_, bcRFF_IMPORTER ),
				new ReField( "F_", &TestBase::F_, bcRFF_IMPORTER ),
				new ReField( "G_", &TestBase::G_, bcRFF_IMPORTER ),
				new ReField( "H_", &TestBase::H_, bcRFF_IMPORTER ),
			};
	
			ReRegisterClass< TestBase >( Fields );

			IsRegistered = true;
		}
	}

	class TestBasePointers
	{
	public:
		REFLECTION_DECLARE_BASE_NOAUTOREG( TestBasePointers );

		TestBasePointers()
		{
		}

		~TestBasePointers()
		{
			delete A_;
			delete B_;
		}

		bool operator == ( const TestBasePointers& Other ) const
		{
			bool RetVal = true;
			if( A_ && Other.A_ )
			{
				RetVal &= *A_ == *Other.A_;
			}
			else
			{
				RetVal &= A_ == Other.A_;
			}
			if( B_ && Other.B_ )
			{
				RetVal &= *B_ == *Other.B_;
			}
			else
			{
				RetVal &= B_ == Other.B_;
			}
			return RetVal;
		}

		TestBase* A_ = nullptr;
		TestBase* B_ = nullptr;
	};

	REFLECTION_DEFINE_BASE( TestBasePointers );

	void TestBasePointers::StaticRegisterClass()
	{
		static bool IsRegistered = false;
		if( IsRegistered == false )
		{
			ReField* Fields[] = 
			{
				new ReField( "A_", &TestBasePointers::A_, bcRFF_IMPORTER | bcRFF_OWNER ),
				new ReField( "B_", &TestBasePointers::B_, bcRFF_IMPORTER | bcRFF_OWNER ),
			};
	
			ReRegisterClass< TestBasePointers >( Fields );

			IsRegistered = true;
		}
	}

	class TestBasePointers2
	{
	public:
		REFLECTION_DECLARE_BASE_NOAUTOREG( TestBasePointers2 );

		TestBasePointers2()
		{
		}

		~TestBasePointers2()
		{
			delete A_;
			delete B_;
			A2_ = nullptr;
			B2_ = nullptr;
		}

		bool operator == ( const TestBasePointers2& Other ) const
		{
			bool RetVal = true;
			if( A_ && Other.A_ )
			{
				RetVal &= *A_ == *Other.A_;
			}
			else
			{
				RetVal &= A_ == Other.A_;
			}
			if( B_ && Other.B_ )
			{
				RetVal &= *B_ == *Other.B_;
			}
			else
			{
				RetVal &= B_ == Other.B_;
			}
			if( A2_ && Other.A2_ )
			{
				RetVal &= *A2_ == *Other.A2_;
			}
			else
			{
				RetVal &= A2_ == Other.A2_;
			}
			if( B2_ && Other.B2_ )
			{
				RetVal &= *B2_ == *Other.B2_;
			}
			else
			{
				RetVal &= B2_ == Other.B2_;
			}
			return RetVal;
		}

		TestBase* A_ = nullptr;
		TestBase* B_ = nullptr;
		TestBase* A2_ = nullptr;
		TestBase* B2_ = nullptr;
	};

	REFLECTION_DEFINE_BASE( TestBasePointers2 );

	void TestBasePointers2::StaticRegisterClass()
	{
		static bool IsRegistered = false;
		if( IsRegistered == false )
		{
			ReField* Fields[] = 
			{
				new ReField( "A_", &TestBasePointers2::A_, bcRFF_IMPORTER | bcRFF_OWNER ),
				new ReField( "B_", &TestBasePointers2::B_, bcRFF_IMPORTER | bcRFF_OWNER ),
				new ReField( "A2_", &TestBasePointers2::A2_, bcRFF_IMPORTER | bcRFF_OWNER ),
				new ReField( "B2_", &TestBasePointers2::B2_, bcRFF_IMPORTER | bcRFF_OWNER ),
			};
	
			ReRegisterClass< TestBasePointers2 >( Fields );

			IsRegistered = true;
		}
	}
}


TEST_CASE( "Serialisation-JsonWriter-Base" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();

	// Create object to save.
	TestBase JsonWriterTestBase_;
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBase_, JsonWriterTestBase_.getClass() );

	INFO( Writer.getOutput() );

	const auto& Value = Writer.getValue();

	// Basic checks.
	REQUIRE( Value.type() == Json::objectValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].type() == Json::intValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].asUInt() == SERIALISER_VERSION );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].type() == Json::arrayValue );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].size() == 1 );
	REQUIRE( Value[ SeISerialiser::RootIDString ].type() == Json::stringValue );

	// Object check.
	const auto& Object = Value[ SeISerialiser::ObjectsString ][ 0 ];
	REQUIRE( Object.type() == Json::objectValue );

	// Check members.
	JsonWriterTestBase_.test( Object );
}


TEST_CASE( "Serialisation-JsonWriter-Base-Pointers" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();
	TestBasePointers::StaticRegisterClass();

	// Create object to save.
	TestBasePointers JsonWriterTestBasePointers_;
	JsonWriterTestBasePointers_.A_ = new TestBase( false );
	JsonWriterTestBasePointers_.B_ = new TestBase( true );
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBasePointers_, JsonWriterTestBasePointers_.getClass() );

	INFO( Writer.getOutput() );

	const auto& Value = Writer.getValue();

	// Basic checks.
	REQUIRE( Value.type() == Json::objectValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].type() == Json::intValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].asUInt() == SERIALISER_VERSION );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].type() == Json::arrayValue );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].size() == 3 );
	REQUIRE( Value[ SeISerialiser::RootIDString ].type() == Json::stringValue );

	// Object check.
	const auto& Object = Value[ SeISerialiser::ObjectsString ][ 0 ];
	REQUIRE( Object.type() == Json::objectValue );

	// Check of the contained objects.
	{
		const auto& Object = Value[ SeISerialiser::ObjectsString ][ 1 ];
		REQUIRE( Object.type() == Json::objectValue );

		TestBase TestBaseObject( false );
		TestBaseObject.test( Object );
	}

	{
		const auto& Object = Value[ SeISerialiser::ObjectsString ][ 2 ];
		REQUIRE( Object.type() == Json::objectValue );

		TestBase TestBaseObject( true );
		TestBaseObject.test( Object );
	}
}


TEST_CASE( "Serialisation-JsonWriter-Base-Pointers-Duplicated" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();
	TestBasePointers2::StaticRegisterClass();

	// Create object to save.
	TestBasePointers2 JsonWriterTestBasePointers_;
	JsonWriterTestBasePointers_.A_ = new TestBase( false );
	JsonWriterTestBasePointers_.B_ = new TestBase( true );
	JsonWriterTestBasePointers_.A2_ = JsonWriterTestBasePointers_.A_;
	JsonWriterTestBasePointers_.B2_ = JsonWriterTestBasePointers_.B_;

	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBasePointers_, JsonWriterTestBasePointers_.getClass() );

	INFO( Writer.getOutput() );

	const auto& Value = Writer.getValue();

	// Basic checks.
	REQUIRE( Value.type() == Json::objectValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].type() == Json::intValue );
	REQUIRE( Value[ SeISerialiser::SerialiserVersionString ].asUInt() == SERIALISER_VERSION );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].type() == Json::arrayValue );
	REQUIRE( Value[ SeISerialiser::ObjectsString ].size() == 3 );
	REQUIRE( Value[ SeISerialiser::RootIDString ].type() == Json::stringValue );

	// Object check.
	const auto& Object = Value[ SeISerialiser::ObjectsString ][ 0 ];
	REQUIRE( Object.type() == Json::objectValue );

	// Check of the contained objects.
	{
		const auto& Object = Value[ SeISerialiser::ObjectsString ][ 1 ];
		REQUIRE( Object.type() == Json::objectValue );

		TestBase TestBaseObject( false );
		TestBaseObject.test( Object );
	}

	{
		const auto& Object = Value[ SeISerialiser::ObjectsString ][ 2 ];
		REQUIRE( Object.type() == Json::objectValue );

		TestBase TestBaseObject( true );
		TestBaseObject.test( Object );
	}
}


TEST_CASE( "Serialisation-JsonReader-Base" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();

	// Create object to save.
	TestBase JsonWriterTestBase_;
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBase_, JsonWriterTestBase_.getClass() );

	INFO( Writer.getOutput() );

	// Read Json in.
	SeJsonReader Reader( &ObjectCodec );
	TestBase Object( true );
	Reader.setRootValue( Writer.getValue() );
	Reader.serialise( &Object, Object.getClass() );

	REQUIRE( Object == JsonWriterTestBase_ );

}

TEST_CASE( "Serialisation-JsonReader-Base-Pointers" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();
	TestBasePointers::StaticRegisterClass();

	// Create object to save.
	TestBasePointers JsonWriterTestBasePointers_;
	JsonWriterTestBasePointers_.A_ = new TestBase( false );
	JsonWriterTestBasePointers_.B_ = new TestBase( true );
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBasePointers_, JsonWriterTestBasePointers_.getClass() );

	INFO( Writer.getOutput() );

	// Read Json in.
	SeJsonReader Reader( &ObjectCodec );
	TestBasePointers Object;
	Reader.setRootValue( Writer.getValue() );
	Reader.serialise( &Object, Object.getClass() );

	REQUIRE( Object == JsonWriterTestBasePointers_ );

}

TEST_CASE( "Serialisation-JsonReader-Base-Pointers-Duplicated" )
{
	using namespace TestData;

	// Register.
	TestBase::StaticRegisterClass();
	TestBasePointers2::StaticRegisterClass();

	// Create object to save.
	TestBasePointers2 JsonWriterTestBasePointers_;
	JsonWriterTestBasePointers_.A_ = new TestBase( false );
	JsonWriterTestBasePointers_.B_ = new TestBase( true );
	JsonWriterTestBasePointers_.A2_ = JsonWriterTestBasePointers_.A_;
	JsonWriterTestBasePointers_.B2_ = JsonWriterTestBasePointers_.B_;
	
	// Write Json out.
	ObjectCodecBasic ObjectCodec;
	SeJsonWriter Writer( &ObjectCodec );
	Writer.serialise( &JsonWriterTestBasePointers_, JsonWriterTestBasePointers_.getClass() );

	INFO( Writer.getOutput() );

	// Read Json in.
	SeJsonReader Reader( &ObjectCodec );
	TestBasePointers2 Object;
	Reader.setRootValue( Writer.getValue() );
	Reader.serialise( &Object, Object.getClass() );

	REQUIRE( Object == JsonWriterTestBasePointers_ );
	REQUIRE( Object.A_ == Object.A2_ );
	REQUIRE( Object.B_ == Object.B2_ );
}

#endif // !PSY_PRODUCTION

//////////////////////////////////////////////////////////////////////////
// Statics
const char* SeISerialiser::SerialiserVersionString = "$SerialiserVersion";
const char* SeISerialiser::RootIDString = "$RootID";
const char* SeISerialiser::ObjectsString = "$Objects";
const char* SeISerialiser::ClassString = "$Class";
const char* SeISerialiser::IDString = "$ID";
const char* SeISerialiser::FieldString = "$Field";
const char* SeISerialiser::ValueString = "$Value";
