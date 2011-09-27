//
//  BcDebug.mm
//  TestBed
//
//  Created by Neil Richardson on 25/09/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
@interface BcMessageBoxObject : NSObject
{
}
- (NSInteger)display:(NSString*) title
			 message: (NSString*) message 
			 button0: (NSString*) button0 
			 button1: (NSString*) button1 
			 button2: (NSString*) button2;
- (void)displayInternal:(NSMutableDictionary*) dict;
@end

@implementation BcMessageBoxObject
- (NSInteger)display:(NSString*) title			
		message: (NSString*) message 
		button0: (NSString*) button0 
		button1: (NSString*) button1 
		button2: (NSString*) button2
{
	NSMutableDictionary* dict = [[NSMutableDictionary alloc]init];
		
	NSAlert* alert = [NSAlert alertWithMessageText:title
									 defaultButton:button0
								   alternateButton:button1
									   otherButton:button2
						 informativeTextWithFormat:message];
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

BcU32 BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, const BcChar* pButton0, const BcChar* pButton1, const BcChar* pButton2 )
{
	BcU32 RetVal = 0;
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSString* title = [[NSString alloc] initWithCString: pTitle encoding: [NSString defaultCStringEncoding]];
	NSString* message = [[NSString alloc] initWithCString: pMessage encoding: [NSString defaultCStringEncoding]];

	NSString* button0 = pButton0 != NULL ? [[NSString alloc] initWithCString: pButton0 encoding: [NSString defaultCStringEncoding]] : nil;
	NSString* button1 = pButton1 != NULL ? [[NSString alloc] initWithCString: pButton1 encoding: [NSString defaultCStringEncoding]] : nil;
	NSString* button2 = pButton2 != NULL ? [[NSString alloc] initWithCString: pButton2 encoding: [NSString defaultCStringEncoding]] : nil;
	
	BcMessageBoxObject* messageBox = [BcMessageBoxObject new];
	NSInteger selection = [messageBox display: title
									  message: message
									  button0: button0
									  button1: button1
									  button2: button2];
	
	// Determine which button was selected.
	switch( selection )
	{
		case 0:	// alternate (1)
			RetVal = 1;
			break;
		case 1: // default (0)
			RetVal = 0;
			break;
		case -1: // other (2)
			RetVal = 2;
			break;
		default:
			break;
	}
	
	[pool drain];
	
	return RetVal;
}