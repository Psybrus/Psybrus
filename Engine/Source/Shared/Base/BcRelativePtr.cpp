#include "Base/BcRelativePtr.h"

//////////////////////////////////////////////////////////////////////////
// BcRelativePtr_UnitTest
void BcRelativePtr_UnitTest()
{
#pragma pack(1)
	struct
	{
		BcRelativePtrU8< BcU8 > PtrU8;
		BcRelativePtrU16< BcU8 > PtrU16;
		BcRelativePtrU32< BcU8 > PtrU32;
		BcRelativePtrS8< BcU8 > PtrS8;
		BcRelativePtrS16< BcU8 > PtrS16;
		BcRelativePtrS32< BcU8 > PtrS32;

		BcU8 FailU8;
		BcRelativePtrU8< BcU8 > FailPtrU8;
#if PLATFORM_WINDOWS
		BcU8 FailU8Max[ 255 + 1 ];
#else
		BcU8 FailU8Max[ std::numeric_limits< BcU8 >::max() + 1 ];
#endif

		BcU8 FailU16;
		BcRelativePtrU16< BcU8 > FailPtrU16;
#if PLATFORM_WINDOWS
		BcU8 FailU16Max[ 65535 + 1 ];
#else
		BcU8 FailU16Max[ std::numeric_limits< BcU16 >::max() + 1 ];
#endif
		BcU8 FailU32;
		BcRelativePtrU32< BcU8 > FailPtrU32;

#if PLATFORM_WINDOWS
		BcU8 FailS8[ 128 + 1 ];
#else
		BcU8 FailS8[ -std::numeric_limits< BcS8 >::min() + 1 ];
#endif
		BcRelativePtrS8< BcU8 > FailPtrS8;

#if PLATFORM_WINDOWS
		BcU8 FailS16[ 32768 + 1 ];
#else
		BcU8 FailS16[ -std::numeric_limits< BcS16 >::min() + 1 ];
#endif
		BcRelativePtrS16< BcU8 > FailPtrS16;

	} Ptrs;
#pragma pack()
	
	// Check all initialise to nullptr.
	BcUnitTest( Ptrs.PtrU8.get() == nullptr );
	BcUnitTest( Ptrs.PtrU16.get() == nullptr );
	BcUnitTest( Ptrs.PtrU32.get() == nullptr );
	BcUnitTest( Ptrs.PtrS8.get() == nullptr );
	BcUnitTest( Ptrs.PtrS16.get() == nullptr );
	BcUnitTest( Ptrs.PtrS32.get() == nullptr );

	// Check all will successfully point to themselves.
	Ptrs.PtrU8.reset( (BcU8*)&Ptrs.PtrU8 );
	Ptrs.PtrU16.reset( (BcU8*)&Ptrs.PtrU16 );
	Ptrs.PtrU32.reset( (BcU8*)&Ptrs.PtrU32 );
	Ptrs.PtrS8.reset( (BcU8*)&Ptrs.PtrS8 );
	Ptrs.PtrS16.reset( (BcU8*)&Ptrs.PtrS16 );
	Ptrs.PtrS32.reset( (BcU8*)&Ptrs.PtrS32 );

	BcUnitTest( Ptrs.PtrU8.offset() == 0 );
	BcUnitTest( Ptrs.PtrU16.offset() == 0 );
	BcUnitTest( Ptrs.PtrU32.offset() == 0 );
	BcUnitTest( Ptrs.PtrS8.offset() == 0 );
	BcUnitTest( Ptrs.PtrS16.offset() == 0 );
	BcUnitTest( Ptrs.PtrS32.offset() == 0 );

	BcUnitTest( Ptrs.PtrU8.get() == (BcU8*)&Ptrs.PtrU8 );
	BcUnitTest( Ptrs.PtrU16.get() == (BcU8*)&Ptrs.PtrU16 );
	BcUnitTest( Ptrs.PtrU32.get() == (BcU8*)&Ptrs.PtrU32 );
	BcUnitTest( Ptrs.PtrS8.get() == (BcU8*)&Ptrs.PtrS8 );
	BcUnitTest( Ptrs.PtrS16.get() == (BcU8*)&Ptrs.PtrS16 );
	BcUnitTest( Ptrs.PtrS32.get() == (BcU8*)&Ptrs.PtrS32 );

	// Check all will successfully point elsewhere.
	Ptrs.PtrU8.reset( (BcU8*)&Ptrs.PtrS8 );
	Ptrs.PtrU16.reset( (BcU8*)&Ptrs.PtrS16 );
	Ptrs.PtrU32.reset( (BcU8*)&Ptrs.PtrS32 );
	Ptrs.PtrS8.reset( (BcU8*)&Ptrs.PtrU8 );
	Ptrs.PtrS16.reset( (BcU8*)&Ptrs.PtrU16 );
	Ptrs.PtrS32.reset( (BcU8*)&Ptrs.PtrU32 );

	BcUnitTest( Ptrs.PtrU8.get() == (BcU8*)&Ptrs.PtrS8 );
	BcUnitTest( Ptrs.PtrU16.get() == (BcU8*)&Ptrs.PtrS16 );
	BcUnitTest( Ptrs.PtrU32.get() == (BcU8*)&Ptrs.PtrS32 );
	BcUnitTest( Ptrs.PtrS8.get() == (BcU8*)&Ptrs.PtrU8 );
	BcUnitTest( Ptrs.PtrS16.get() == (BcU8*)&Ptrs.PtrU16 );
	BcUnitTest( Ptrs.PtrS32.get() == (BcU8*)&Ptrs.PtrU32 );

	// Test to ensure we are asserting invalid conditions.
	bool GotAssert = false;
	BcAssertScopedHandler ScopedHandler( 
		[ &GotAssert ]( const BcChar* MessageBuffer, const BcChar* File, int Line )
		{
			BcPrintf( "Caught assert: \"%s\" in %s on line %u.\n\nDo you wish to break?\n", MessageBuffer, File, Line );
			GotAssert = true;
			return BcFalse;
		} );

	// Check unsigned ones pointing prior to themselves.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8 );
	BcUnitTest( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16 );
	BcUnitTest( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU32.reset( (BcU8*)&Ptrs.FailU32 );
	BcUnitTest( GotAssert );

	// Check we don't assert pointing to self.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailPtrU8 );
	BcUnitTest( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8Max[ 254 - sizeof( Ptrs.FailPtrU8 ) ] );
	BcUnitTest( GotAssert == false );

	// Check MAX+1 bytes.
	GotAssert = false;
	Ptrs.FailPtrU8.reset( (BcU8*)&Ptrs.FailU8Max[ 255 - sizeof( Ptrs.FailPtrU8 ) ] );
	BcUnitTest( GotAssert );

	// Check self.
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailPtrU16 );
	BcUnitTest( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16Max[ 65534 - sizeof( Ptrs.FailPtrU16 ) ] );
	BcUnitTest( GotAssert == false );

	// Check MAX + 1 bytes
	GotAssert = false;
	Ptrs.FailPtrU16.reset( (BcU8*)&Ptrs.FailU16Max[ 65535 - sizeof( Ptrs.FailPtrU16 ) ] );
	BcUnitTest( GotAssert );

	// Cehck self.
	GotAssert = false;
	Ptrs.FailPtrU32.reset( (BcU8*)&Ptrs.FailPtrU32 );
	BcUnitTest( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS8.reset( (BcU8*)&Ptrs.FailS8[ 0 ] );
	BcUnitTest( GotAssert );

	// Check MIN. 
	GotAssert = false;
	Ptrs.FailPtrS8.reset( (BcU8*)&Ptrs.FailS8[ 1 ] );
	BcUnitTest( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS16.reset( (BcU8*)&Ptrs.FailS16[ 0 ] );
	BcUnitTest( GotAssert );

	// Check MIN.
	GotAssert = false;
	Ptrs.FailPtrS16.reset( (BcU8*)&Ptrs.FailS16[ 1 ] );
	BcUnitTest( GotAssert == false );

}
