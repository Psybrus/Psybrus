/**************************************************************************
*
* File:		OsViewOSX.h
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSVIEWOSX_H__
#define __OSVIEWOSX_H__

#include "OsEvents.h"

//////////////////////////////////////////////////////////////////////////
// OsViewOSX_Interface
class OsViewOSX_Interface
{
public:
	struct TInputEvent
	{
		OsEvents		EventID_;
		BcU16			DeviceID_;
		
		// Mouse.
		BcS16			MouseX_;
		BcS16			MouseY_;
		BcS16			ButtonCode_;
		
		// Keyboard.
		BcU16			KeyCode_;
	};
	
public:
	static void MakeContextCurrent();
	static void FlushBuffer();
	static BcBool GetInputEvent( TInputEvent& Event );
	static void AddInputEvent( TInputEvent& Event );
};

#if __OBJC__
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLContext.h>
#include <AGL/agl.h>

@interface OsViewOSX : NSOpenGLView
{
}

+ (NSOpenGLPixelFormat*) basicPixelFormat;

- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;

- (void) mouseDown:(NSEvent *)theEvent;
- (void) rightMouseDown:(NSEvent *)theEvent;
- (void) otherMouseDown:(NSEvent *)theEvent;
- (void) mouseUp:(NSEvent *)theEvent;
- (void) rightMouseUp:(NSEvent *)theEvent;
- (void) otherMouseUp:(NSEvent *)theEvent;
- (void) mouseDragged:(NSEvent *)theEvent;
- (void) scrollWheel:(NSEvent *)theEvent;
- (void) rightMouseDragged:(NSEvent *)theEvent;
- (void) otherMouseDragged:(NSEvent *)theEvent;

- (void) prepareOpenGL;
- (void) update;        // moved or resized

- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;

- (id) initWithCoder:(NSCoder *)coder;
- (void) awakeFromNib;

@end

#endif //__OBJC__
#endif //__OSVIEWOSX_H__
