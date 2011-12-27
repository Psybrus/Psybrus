/**************************************************************************
*
* File:		RsContextGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsContextGL.h"
#include "OsClient.h"

//////////////////////////////////////////////////////////////////////////
// Statics
#if PLATFORM_WINDOWS
HDC RsContextGL::LastWindowDC_ = NULL;
HGLRC RsContextGL::LastWindowRC_ = NULL;
#endif

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextGL::RsContextGL( OsClient* pClient, RsContextGL* pParent ):
	RsContext( pClient ),
	pParent_( pParent )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextGL::~RsContextGL()
{

}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextGL::getWidth() const
{
	return pClient_->getWidth();
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextGL::getHeight() const
{
	return pClient_->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// makeCurrent
void RsContextGL::makeCurrent()
{
	if( LastWindowDC_ != WindowDC_ || LastWindowRC_ != WindowRC_ )
	{
		// Free up previous.
		wglMakeCurrent( NULL, NULL );

		// Make current.
		wglMakeCurrent( WindowDC_, WindowRC_ );

		// Cache values to prevent calling multiple times.
		LastWindowDC_ = WindowDC_;
		LastWindowRC_ = WindowRC_;
	}
}

//////////////////////////////////////////////////////////////////////////
// swapBuffers
void RsContextGL::swapBuffers()
{
	glFlush();
	::SwapBuffers( WindowDC_ );
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsContextGL::create()
{
#if PLATFORM_WINDOWS
	// Get client device handle.
	WindowDC_ = (HDC)pClient_->getDeviceHandle();

	// Pixel format.
	static  PIXELFORMATDESCRIPTOR pfd =                 // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
		3,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		32,												// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		24,												// 24 bit Z-Buffer (Depth Buffer)
		0,												// No Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};
	
	GLuint PixelFormat = 0;
	if ( !(PixelFormat = ::ChoosePixelFormat( WindowDC_, &pfd ) ) )
	{
		BcPrintf( "Can't create pixel format.\n" );
	}
	
	if( !::SetPixelFormat( WindowDC_, PixelFormat, &pfd ) )               // Are We Able To Set The Pixel Format?
	{
	    BcPrintf( "Can't Set The PixelFormat." );
	}
	
	// Create a rendering context.
	WindowRC_ = wglCreateContext( WindowDC_ );
	BcAssertMsg( WindowRC_ != NULL, "RsCoreImplGL: Render context is NULL!" );

	// If we have a parent, we need to share lists.
	if( pParent_ != NULL )
	{
		// Make parent current.
		pParent_->makeCurrent();

		// Share parent's lists with this context.
		BOOL Result = wglShareLists( pParent_->WindowRC_, WindowRC_ );
		BcAssertMsg( Result != BcFalse, "Unable to share lists." );
	}

	// Do the switch to this context.
	makeCurrent();

	// Clear screen and flip.
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
	// Swap buffers.
	swapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsContextGL::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsContextGL::destroy()
{
#if PLATFORM_WINDOWS
	// Destroy rendering context.
	wglMakeCurrent( WindowDC_, NULL );
	wglDeleteContext( WindowRC_ );
#endif
}


#endif
