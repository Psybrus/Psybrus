/**************************************************************************
*
* File:		OsAppDelegateOSX.h
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSAPPDELEGATEOSX_H__
#define __OSAPPDELEGATEOSX_H__

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

//////////////////////////////////////////////////////////////////////////
// OsAppDelegateOSX
@interface OsAppDelegateOSX : NSObject
{
}
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;

- (void)windowWillClose:(NSNotification*)aNotification;

@end

#endif //__OBJC__
#endif //__OSAPPDELEGATEOSX_H__