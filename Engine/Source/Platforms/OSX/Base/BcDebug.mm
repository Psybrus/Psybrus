#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#include "Base/BcDebug.h"
#include "Base/BcLog.h"

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
@interface BcMessageBoxObject : NSObject
{
}
- (NSInteger)display:(NSString*) title
			 message: (NSString*) message 
			 button0: (NSString*) button0 
			 button1: (NSString*) button1 
			 button2: (NSString*) button2
				icon: (BcMessageBoxIcon) icon;

- (void)displayInternal:(NSMutableDictionary*) dict;
@end

@implementation BcMessageBoxObject
- (NSInteger)display:(NSString*) title			
		message: (NSString*) message 
		button0: (NSString*) button0 
		button1: (NSString*) button1 
		button2: (NSString*) button2
		   icon: (BcMessageBoxIcon) icon
{
	NSMutableDictionary* dict = [[NSMutableDictionary alloc]init];
		
	NSAlert* alert = [NSAlert alertWithMessageText:title
									 defaultButton:button0
								   alternateButton:button1
									   otherButton:button2
						 informativeTextWithFormat:message];
	switch( icon )
	{
		case bcMBI_WARNING:
		case bcMBI_ERROR:
			[alert setAlertStyle:NSCriticalAlertStyle];
			break;
		default:
			[alert setAlertStyle:NSInformationalAlertStyle];
			break;
	}
	[dict setObject:alert forKey:@"alert"];
	[self performSelectorOnMainThread: @selector(displayInternal:)
							withObject: dict
						    waitUntilDone: YES];
	NSNumber* selection = [dict objectForKey:@"selection"];
	return [selection integerValue];
}

- (void)displayInternal:(NSMutableDictionary*) dict
{
	NSAlert* alert = [dict objectForKey:@"alert"];
	NSNumber* retVal = [[NSNumber alloc] initWithInteger: [alert runModal]];
	[dict setObject:retVal forKey:@"selection"];
}
@end

BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type, BcMessageBoxIcon Icon )
{
	PSY_LOG( "%s: %s\n", pTitle, pMessage );

	BcMessageBoxReturn RetVal = bcMBR_OK;

	if( BcIsGameThread() )
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSString* title = [[NSString alloc] initWithCString: pTitle encoding: NSUTF8StringEncoding];
		NSString* message = [[NSString alloc] initWithCString: pMessage encoding: NSUTF8StringEncoding];
		
		NSString* button0 = nil;
		NSString* button1 = nil;
		NSString* button2 = nil;
		
		switch( Type )
		{
			case bcMBT_OK:
				button0 = @"OK";
				break;
			case bcMBT_OKCANCEL:
				button0 = @"OK";
				button1 = @"Cancel";
				break;
			case bcMBT_YESNO:
				button0 = @"Yes";
				button2 = @"No";
				break;
			case bcMBT_YESNOCANCEL:
				button0 = @"Yes";
				button1 = @"Cancel";
				button2 = @"No";
				break;
		}
		
		BcMessageBoxObject* messageBox = [BcMessageBoxObject new];
		NSInteger selection = [messageBox display: title
										  message: message
										  button0: button0
										  button1: button1
										  button2: button2
											 icon: Icon];
		
		// Determine which button was selected.
		switch( selection )
		{
			case NSAlertAlternateReturn:	// alternate (1)
				RetVal = bcMBR_CANCEL;
				break;
			case NSAlertDefaultReturn: // default (0)
				RetVal = bcMBR_YES;
				break;
			case NSAlertOtherReturn: // other (2)
				RetVal = bcMBR_NO;
				break;
			default:
				break;
		}
		[pool drain];	
	}
	return RetVal;
}