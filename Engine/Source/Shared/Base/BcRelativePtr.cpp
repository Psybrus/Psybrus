#include "Base/BcRelativePtr.h"

//////////////////////////////////////////////////////////////////////////
// BcRelativePtr_UnitTest
void BcRelativePtr_UnitTest()
{
#pragma pack(1)
	struct
	{
		BcRelativePtrU8< void > PtrU8;
		BcRelativePtrU16< void > PtrU16;
		BcRelativePtrU32< void > PtrU32;
		BcRelativePtrS8< void > PtrS8;
		BcRelativePtrS16< void > PtrS16;
		BcRelativePtrS32< void > PtrS32;

		BcU8 FailU8;
		BcRelativePtrU8< void > FailPtrU8;
		BcU8 FailU8Max[ std::numeric_limits< BcU8 >::max() + 1 ];

		BcU8 FailU16;
		BcRelativePtrU16< void > FailPtrU16;
		BcU8 FailU16Max[ std::numeric_limits< BcU16 >::max() + 1 ];

		BcU8 FailU32;
		BcRelativePtrU32< void > FailPtrU32;

		BcU8 FailS8[ -std::numeric_limits< BcS8 >::min() + 1 ];
		BcRelativePtrS8< void > FailPtrS8;

		BcU8 FailS16[ -std::numeric_limits< BcS16 >::min() + 1 ];
		BcRelativePtrS16< void > FailPtrS16;

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
	Ptrs.PtrU8 = &Ptrs.PtrU8;
	Ptrs.PtrU16 = &Ptrs.PtrU16;
	Ptrs.PtrU32 = &Ptrs.PtrU32;
	Ptrs.PtrS8 = &Ptrs.PtrS8;
	Ptrs.PtrS16 = &Ptrs.PtrS16;
	Ptrs.PtrS32 = &Ptrs.PtrS32;

	BcUnitTest( Ptrs.PtrU8.offset() == 0 );
	BcUnitTest( Ptrs.PtrU16.offset() == 0 );
	BcUnitTest( Ptrs.PtrU32.offset() == 0 );
	BcUnitTest( Ptrs.PtrS8.offset() == 0 );
	BcUnitTest( Ptrs.PtrS16.offset() == 0 );
	BcUnitTest( Ptrs.PtrS32.offset() == 0 );

	BcUnitTest( Ptrs.PtrU8.get() == &Ptrs.PtrU8 );
	BcUnitTest( Ptrs.PtrU16.get() == &Ptrs.PtrU16 );
	BcUnitTest( Ptrs.PtrU32.get() == &Ptrs.PtrU32 );
	BcUnitTest( Ptrs.PtrS8.get() == &Ptrs.PtrS8 );
	BcUnitTest( Ptrs.PtrS16.get() == &Ptrs.PtrS16 );
	BcUnitTest( Ptrs.PtrS32.get() == &Ptrs.PtrS32 );

	// Check all will successfully point elsewhere.
	Ptrs.PtrU8 = &Ptrs.PtrS8;
	Ptrs.PtrU16 = &Ptrs.PtrS16;
	Ptrs.PtrU32 = &Ptrs.PtrS32;
	Ptrs.PtrS8 = &Ptrs.PtrU8;
	Ptrs.PtrS16 = &Ptrs.PtrU16;
	Ptrs.PtrS32 = &Ptrs.PtrU32;

	BcUnitTest( Ptrs.PtrU8.get() == &Ptrs.PtrS8 );
	BcUnitTest( Ptrs.PtrU16.get() == &Ptrs.PtrS16 );
	BcUnitTest( Ptrs.PtrU32.get() == &Ptrs.PtrS32 );
	BcUnitTest( Ptrs.PtrS8.get() == &Ptrs.PtrU8 );
	BcUnitTest( Ptrs.PtrS16.get() == &Ptrs.PtrU16 );
	BcUnitTest( Ptrs.PtrS32.get() == &Ptrs.PtrU32 );

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
	Ptrs.FailPtrU8 = &Ptrs.FailU8;
	BcUnitTest( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU16 = &Ptrs.FailU16;
	BcUnitTest( GotAssert );

	GotAssert = false;
	Ptrs.FailPtrU32 = &Ptrs.FailU32;
	BcUnitTest( GotAssert );

	// Check we don't assert pointing to self.
	GotAssert = false;
	Ptrs.FailPtrU8 = &Ptrs.FailPtrU8;
	BcUnitTest( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU8 = &Ptrs.FailU8Max[ 254 - sizeof( Ptrs.FailPtrU8 ) ];
	BcUnitTest( GotAssert == false );

	// Check MAX+1 bytes.
	GotAssert = false;
	Ptrs.FailPtrU8 = &Ptrs.FailU8Max[ 255 - sizeof( Ptrs.FailPtrU8 ) ];
	BcUnitTest( GotAssert );

	// Check self.
	GotAssert = false;
	Ptrs.FailPtrU16 = &Ptrs.FailPtrU16;
	BcUnitTest( GotAssert == false );

	// Check MAX bytes.
	GotAssert = false;
	Ptrs.FailPtrU16 = &Ptrs.FailU16Max[ 65534 - sizeof( Ptrs.FailPtrU16 ) ];
	BcUnitTest( GotAssert == false );

	// Check MAX + 1 bytes
	GotAssert = false;
	Ptrs.FailPtrU16 = &Ptrs.FailU16Max[ 65535 - sizeof( Ptrs.FailPtrU16 ) ];
	BcUnitTest( GotAssert );

	// Cehck self.
	GotAssert = false;
	Ptrs.FailPtrU32 = &Ptrs.FailPtrU32;
	BcUnitTest( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS8 = &Ptrs.FailS8[ 0 ];
	BcUnitTest( GotAssert );

	// Check MIN. 
	GotAssert = false;
	Ptrs.FailPtrS8 = &Ptrs.FailS8[ 1 ];
	BcUnitTest( GotAssert == false );

	// Check MIN - 1. 
	GotAssert = false;
	Ptrs.FailPtrS16 = &Ptrs.FailS16[ 0 ];
	BcUnitTest( GotAssert );

	// Check MIN.
	GotAssert = false;
	Ptrs.FailPtrS16 = &Ptrs.FailS16[ 1 ];
	BcUnitTest( GotAssert == false );

}
