/**************************************************************************
*
* File:		OsViewOSX.mm
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#import "OsViewOSX.h"

#include "BcMutex.h"
#include "BcScopedLock.h"
#include "BcDelegate.h"
#include "OsCore.h"
#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// OsViewOSX
//////////////////////////////////////////////////////////////////////////

static OsViewOSX* OsViewOSX_Impl = NULL;

static BcMutex GInputEventLock_;
static std::list< OsViewOSX_Interface::TInputEvent > GInputEventList_;

//////////////////////////////////////////////////////////////////////////
// MakeContextCurrent
//static
void OsViewOSX_Interface::MakeContextCurrent()
{
	[[OsViewOSX_Impl openGLContext] makeCurrentContext];
}

//////////////////////////////////////////////////////////////////////////
// FlushBuffer
//static
void OsViewOSX_Interface::FlushBuffer()
{
	[[OsViewOSX_Impl openGLContext] flushBuffer];
}

//////////////////////////////////////////////////////////////////////////
// GetInputEvent
//static
BcBool OsViewOSX_Interface::GetInputEvent( TInputEvent& Event )
{
	BcScopedLock< BcMutex > Lock( GInputEventLock_ );

	if( GInputEventList_.size() > 0 )
	{
		Event = *GInputEventList_.begin();
		GInputEventList_.pop_front();
		return BcTrue;
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// AddInputEvent
//static
void OsViewOSX_Interface::AddInputEvent( TInputEvent& Event )
{
	BcScopedLock< BcMutex > Lock( GInputEventLock_ );
	GInputEventList_.push_back( Event );
}

//////////////////////////////////////////////////////////////////////////
// OsViewOSX
//////////////////////////////////////////////////////////////////////////

@implementation OsViewOSX

//////////////////////////////////////////////////////////////////////////
// basicPixelFormat
+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute attributes [] =
	{
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,    // double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24, // 24 bit depth buffer
        (NSOpenGLPixelFormatAttribute)nil
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

//////////////////////////////////////////////////////////////////////////
// resizeGL
- (void) resizeGL
{
    NSRect rectView = [self bounds];
    
	/*
    // ensure camera knows size changed
    if ((camera.viewHeight != rectView.size.height) ||
        (camera.viewWidth != rectView.size.width)) {
        camera.viewHeight = rectView.size.height;
        camera.viewWidth = rectView.size.width;
        
        glViewport (0, 0, camera.viewWidth, camera.viewHeight);
        [self updateProjection];  // update projection matrix
        [self updateInfoString];
    }
	 */
}

//////////////////////////////////////////////////////////////////////////
// keyDown
-(void)keyDown:(NSEvent *)theEvent
{
    int key = [theEvent keyCode];
	OsViewOSX_Interface::TInputEvent Event =
	{
		osEVT_INPUT_KEYDOWN,
		0,
		0, 0, 0,
		key & 0xffff
	};
	OsViewOSX_Interface::AddInputEvent( Event );
}

//////////////////////////////////////////////////////////////////////////
// keyUp
-(void)keyUp:(NSEvent *)theEvent
{
    int key = [theEvent keyCode];
	OsViewOSX_Interface::TInputEvent Event = 
	{
		osEVT_INPUT_KEYUP,
		0,
		0, 0, 0,
		key & 0xffff
	};
	OsViewOSX_Interface::AddInputEvent( Event );
}

//////////////////////////////////////////////////////////////////////////
// mouseDown
- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	BcPrintf( "mouseDown: %f, %f\n", location.x, location.y );
}

//////////////////////////////////////////////////////////////////////////
// rightMouseDown
- (void)rightMouseDown:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	BcPrintf( "rightMouseDown: %f, %f\n", location.x, location.y );
}

//////////////////////////////////////////////////////////////////////////
// otherMouseDown
- (void)otherMouseDown:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	BcPrintf( "otherMouseDown: %f, %f\n", location.x, location.y );
}

//////////////////////////////////////////////////////////////////////////
// mouseUp
- (void)mouseUp:(NSEvent *)theEvent
{

}

//////////////////////////////////////////////////////////////////////////
// rightMouseUp
- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self mouseUp:theEvent];
}

//////////////////////////////////////////////////////////////////////////
// otherMouseUp
- (void)otherMouseUp:(NSEvent *)theEvent
{
    [self mouseUp:theEvent];
}

//////////////////////////////////////////////////////////////////////////
// mouseDragged
- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	//BcPrintf( "mouseDown: %f, %f\n", location.x, location.y );
}

//////////////////////////////////////////////////////////////////////////
// scrollWheel
- (void)scrollWheel:(NSEvent *)theEvent
{
    float wheelDelta = [theEvent deltaX] +[theEvent deltaY] + [theEvent deltaZ];

	//BcPrintf( "scrollWheel: %f\n", wheelDelta );

	if (wheelDelta)
    {
        [self setNeedsDisplay: YES];
    }
}

//////////////////////////////////////////////////////////////////////////
// rightMouseDragged
- (void)rightMouseDragged:(NSEvent *)theEvent
{
    [self mouseDragged: theEvent];
}

//////////////////////////////////////////////////////////////////////////
// otherMouseDragged
- (void)otherMouseDragged:(NSEvent *)theEvent
{
    [self mouseDragged: theEvent];
}

//////////////////////////////////////////////////////////////////////////
// prepareOpenGL
- (void) prepareOpenGL
{
	// Turn on vsync.
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

//////////////////////////////////////////////////////////////////////////
// update
- (void) update
{
    [super update];
}

//////////////////////////////////////////////////////////////////////////
// initWithCoder
- (id) initWithCoder:(NSCoder *)coder;
{
    self = [super initWithCoder: coder];
	OsViewOSX_Impl = self;
	return self;
}

//////////////////////////////////////////////////////////////////////////
// acceptsFirstResponder
- (BOOL)acceptsFirstResponder
{
	return YES;
}

//////////////////////////////////////////////////////////////////////////
// becomeFirstResponder
- (BOOL)becomeFirstResponder
{
	return  YES;
}

//////////////////////////////////////////////////////////////////////////
// resignFirstResponder
- (BOOL)resignFirstResponder
{
	return YES;
}

//////////////////////////////////////////////////////////////////////////
// awakeFromNib
- (void) awakeFromNib
{
	
}


@end