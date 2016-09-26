#include "Base/BcRelativePtr.h"

#include <limits>

//////////////////////////////////////////////////////////////////////////
// Unit tests.
#if !PSY_PRODUCTION
#include <catch.hpp>

#pragma pack(1)
namespace
{
	struct TestPtrs
	{
		BcRelativePtrU8< BcU8 > PtrU8;
		BcRelativePtrU16< BcU8 > PtrU16;
		BcRelativePtrU32< BcU8 > PtrU32;
		BcRelativePtrS8< BcU8 > PtrS8;
		BcRelativePtrS16< BcU8 > PtrS16;
		BcRelativePtrS32< BcU8 > PtrS32;

		BcU8 FailU8;
		BcRelativePtrU8< BcU8 > FailPtrU8;
		BcU8 FailU8Max[ std::numeric_limits< BcU8 >::max() + 1 ];

		BcU8 FailU16;
		BcRelativePtrU16< BcU8 > FailPtrU16;
		BcU8 FailU16Max[ std::numeric_limits< BcU16 >::max() + 1 ];

		BcU8 FailU32;
		BcRelativePtrU32< BcU8 > FailPtrU32;
		BcU8 FailS8[ -std::numeric_limits< BcS8 >::min() + 1 ];

		BcRelativePtrS8< BcU8 > FailPtrS8;
		BcU8 FailS16[ -std::numeric_limits< BcS16 >::min() + 1 ];
		BcRelativePtrS16< BcU8 > FailPtrS16;
	};
}
#pragma pack()

TEST_CASE( "BcRelativePtr-NullInit")
{
	TestPtrs Ptrs;

	// Check all initialise to nullptr.
	REQUIRE( Ptrs.PtrU8.get() == nullptr );
	REQUIRE( Ptrs.PtrU16.get() == nullptr );
	REQUIRE( Ptrs.PtrU32.get() == nullptr );
	REQUIRE( Ptrs.PtrS8.get() == nullptr );
	REQUIRE( Ptrs.PtrS16.get() == nullptr );
	REQUIRE( Ptrs.PtrS32.get() == nullptr );
}

TEST_CASE( "BcRelativePtr-ResetToSelf")
{
	TestPtrs Ptrs;

	// Check all will successfully point to themselves.
	Ptrs.PtrU8.reset( (BcU8*)&Ptrs.PtrU8 );
	Ptrs.PtrU16.reset( (BcU8*)&Ptrs.PtrU16 );
	Ptrs.PtrU32.reset( (BcU8*)&Ptrs.PtrU32 );
	Ptrs.PtrS8.reset( (BcU8*)&Ptrs.PtrS8 );
	Ptrs.PtrS16.reset( (BcU8*)&Ptrs.PtrS16 );
	Ptrs.PtrS32.reset( (BcU8*)&Ptrs.PtrS32 );

	REQUIRE( Ptrs.PtrU8.offset() == 0 );
	REQUIRE( Ptrs.PtrU16.offset() == 0 );
	REQUIRE( Ptrs.PtrU32.offset() == 0 );
	REQUIRE( Ptrs.PtrS8.offset() == 0 );
	REQUIRE( Ptrs.PtrS16.offset() == 0 );
	REQUIRE( Ptrs.PtrS32.offset() == 0 );

	REQUIRE( Ptrs.PtrU8.get() == (BcU8*)&Ptrs.PtrU8 );
	REQUIRE( Ptrs.PtrU16.get() == (BcU8*)&Ptrs.PtrU16 );
	REQUIRE( Ptrs.PtrU32.get() == (BcU8*)&Ptrs.PtrU32 );
	REQUIRE( Ptrs.PtrS8.get() == (BcU8*)&Ptrs.PtrS8 );
	REQUIRE( Ptrs.PtrS16.get() == (BcU8*)&Ptrs.PtrS16 );
	REQUIRE( Ptrs.PtrS32.get() == (BcU8*)&Ptrs.PtrS32 );
}

TEST_CASE( "BcRelativePtr-ResetToElsewhere")
{
	TestPtrs Ptrs;

	// Check all will successfully point elsewhere.
	Ptrs.PtrU8.reset( (BcU8*)&Ptrs.PtrS8 );
	Ptrs.PtrU16.reset( (BcU8*)&Ptrs.PtrS16 );
	Ptrs.PtrU32.reset( (BcU8*)&Ptrs.PtrS32 );
	Ptrs.PtrS8.reset( (BcU8*)&Ptrs.PtrU8 );
	Ptrs.PtrS16.reset( (BcU8*)&Ptrs.PtrU16 );
	Ptrs.PtrS32.reset( (BcU8*)&Ptrs.PtrU32 );

	REQUIRE( Ptrs.PtrU8.get() == (BcU8*)&Ptrs.PtrS8 );
	REQUIRE( Ptrs.PtrU16.get() == (BcU8*)&Ptrs.PtrS16 );
	REQUIRE( Ptrs.PtrU32.get() == (BcU8*)&Ptrs.PtrS32 );
	REQUIRE( Ptrs.PtrS8.get() == (BcU8*)&Ptrs.PtrU8 );
	REQUIRE( Ptrs.PtrS16.get() == (BcU8*)&Ptrs.PtrU16 );
	REQUIRE( Ptrs.PtrS32.get() == (BcU8*)&Ptrs.PtrU32 );
}

TEST_CASE( "BcRelativePtr-TestAssertions")
{
	TestPtrs Ptrs;

	// Test to ensure we are asserting invalid conditions.
	bool GotAssert = false;
	BcAssertScopedHandler ScopedHandler( 
		[ &GotAssert ]( const BcChar* MessageBuffer, const BcChar* File, int Line )
		{
			GotAssert = true;
			return BcFalse;
		} );

	// Check unsigned ones pointing prior to themselves.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8 );
	REQUIRE( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16 );
	REQUIRE( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU32.reset( (BcU8*)&Ptrs.FailU32 );
	REQUIRE( GotAssert );

	// Check we don't assert pointing to self.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailPtrU8 );
	REQUIRE( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8Max[ 254 - sizeof( Ptrs.FailPtrU8 ) ] );
	REQUIRE( GotAssert == false );

	// Check MAX+1 bytes.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8Max[ 255 - sizeof( Ptrs.FailPtrU8 ) ] );
	REQUIRE( GotAssert );

	// Check self.
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailPtrU16 );
	REQUIRE( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16Max[ 65534 - sizeof( Ptrs.FailPtrU16 ) ] );
	REQUIRE( GotAssert == false );

	// Check MAX + 1 bytes
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16Max[ 65535 - sizeof( Ptrs.FailPtrU16 ) ] );
	REQUIRE( GotAssert );

	// Check self.
	GotAssert = false;
	Ptrs.FailPtrU32.reset( (BcU8*)&Ptrs.FailPtrU32 );
	REQUIRE( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS8.reset( (BcU8*)&Ptrs.FailS8[ 0 ] );
	REQUIRE( GotAssert );

	// Check MIN. 
	GotAssert = false;
	Ptrs.FailPtrS8.reset( (BcU8*)&Ptrs.FailS8[ 1 ] );
	REQUIRE( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS16.reset( (BcU8*)&Ptrs.FailS16[ 0 ] );
	REQUIRE( GotAssert );

	// Check MIN.
	GotAssert = false;
	Ptrs.FailPtrS16.reset( (BcU8*)&Ptrs.FailS16[ 1 ] );
	REQUIRE( GotAssert == false );
}

#endif !PSY_PRODUCTION

