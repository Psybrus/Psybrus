/**************************************************************************
*
* File:		OsAppDelegateOSX.mm
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#import "OsAppDelegateOSX.h"

#import <Cocoa/Cocoa.h>
#import <AGL/agl.h>

#include "BcDebug.h"
#include "SysKernel.h"
#include "MainShared.h"

@implementation OsAppDelegateOSX

//////////////////////////////////////////////////////////////////////////
// applicationDidFinishLaunching
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
	// Main shared.
	MainShared();
	
	// Game init.
	extern void PsyGameInit();
	PsyGameInit();
	
	// Run kernel threaded.
	SysKernel::pImpl()->run( BcTrue );
}

//////////////////////////////////////////////////////////////////////////
// applicationShouldTerminateAfterLastWindowClosed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

//////////////////////////////////////////////////////////////////////////
// windowWillClose
- (void)windowWillClose:(NSNotification*)aNotification
{
	// If we are going to terminate and have a kernel around
	// we need to stop.
	if( SysKernel::pImpl() != NULL )
	{
		// Stop kernel.
		SysKernel::pImpl()->stop();
		
		// Join with kernel.
		SysKernel::pImpl()->join();
	}
}

@end
