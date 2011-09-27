#import "OsAppDelegateOSX.h"

#include "MainShared.h"
#include "SysKernel.h"

int main( int argc, char* argv[] )
{
	// Perform unit tests.
	MainUnitTests();
	
	// Set command line params.
	SysArgc_ = (BcU32)argc;
	SysArgv_ = (BcChar**)&argv[0];
	
	// Create kernel.
	new SysKernel();
	
	// Setup creators.
	SYS_REGISTER( "GaCore", GaCore );
	SYS_REGISTER( "RmCore", RmCore );
	SYS_REGISTER( "OsCore", OsCoreImplOSX );
	SYS_REGISTER( "FsCore", FsCoreImplOSX );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );

	// Enter application main.
	return NSApplicationMain(argc,  (const char **) argv);
}

void PsyGameInit()
{
	// Do game init here.
}


/*
 #include <portmidi.h>
 
BcTimer MIDITimer_;
PmTimestamp TimeProc(void* time_info)
{
	return MIDITimer_.time() * 1000.0f; 
}

{
	// PortMIDI test.
	Pm_Initialize();
	
	for(;;)
	{
	int NoofDevices = Pm_CountDevices();
	
	BcPrintf( "Number of MIDI devices: %u\n", NoofDevices );
	
	// Enumerate devices.
	PmDeviceInfo* pDeviceInfos = new PmDeviceInfo[ NoofDevices ];
	
	for( int i = 0; i < NoofDevices; ++i )
	{
		const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo( i );
		BcMemCopy( &pDeviceInfos[ i ], pDeviceInfo, sizeof( PmDeviceInfo ) );
		
		BcPrintf( "Device %u:\n", i );
		BcPrintf( " - Name: %s\n", pDeviceInfo->name );
		BcPrintf( " - Input: %u\n", pDeviceInfo->input );
		BcPrintf( " - Output: %u\n", pDeviceInfo->output );
	}
	
	// Mark the MIDI timers.
	MIDITimer_.mark();
	
	// Open input stream.
	PortMidiStream* pStream = NULL;
	PmError RetVal = Pm_OpenInput( &pStream,
								   0,
								   NULL, 
								   128,
								   TimeProc,
								   NULL );
	
	if( RetVal == pmNoError )
	{
		PmEvent Events[ 128 ];
		for(;;)
		{
			if( Pm_Poll( pStream ) == BcTrue )
			{
				int NoofEvents = Pm_Read( pStream, &Events[ 0 ], 128 );
				
				for( int i = 0; i < NoofEvents; ++i )
				{
					BcU32 MessageAll = Events[ i ].message;
					BcU8 Data0 = ( MessageAll ) & 0xff;
					BcU8 Data1 = ( MessageAll >> 8 ) & 0x7f;
					BcU8 Data2 = ( MessageAll >> 16 ) & 0x7f;
					BcU8 Data3 = ( MessageAll >> 24 ) & 0xff;
					BcU8 Message = ( MessageAll >> 4 ) & 0xf;
					BcU8 Channel = ( MessageAll ) & 0xf;
					
					
					
					BcPrintf( "Message/Channel: 0x%x, 0x%x (0x%x, 0x%x)\n", Message, Channel, Data1, Data2 );
					
				}
			}
			BcYield();
		}
	}
		BcSleep(2.0f);
	}
	
	Pm_Terminate();
	
	return 0;
}
 */

