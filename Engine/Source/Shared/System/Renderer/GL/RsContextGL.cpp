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

#include "System/Renderer/GL/RsContextGL.h"
#include "System/Os/OsClient.h"

#include "Import/Img/Img.h"

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
	pParent_( pParent ),
	ScreenshotRequested_( BcFalse )
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
	if( ScreenshotRequested_ == BcFalse )
	{
		glFlush();
	}
	else
	{
		// Finish all rendering.
		glFinish();
		
		// Read the back buffer.
		glReadBuffer( GL_BACK );
		BcU32* pImageData = new BcU32[ getWidth() * getHeight() ];
		BcU32* pReadImageData = pImageData;
		glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
		
		// Convert to image.
		ImgImage* pImage = new ImgImage();
		pImage->create( getWidth(), getHeight(), NULL );
		
		BcU32 W = getWidth();
		BcU32 H = getHeight();
		for( BcU32 Y = 0; Y < H; ++Y )
		{
			BcU32 RealY = ( H - Y ) - 1;
			for( BcU32 X = 0; X < W; ++X )
			{
				ImgColour* pColour = (ImgColour*)pReadImageData++;
				pImage->setPixel( X, RealY, *pColour );
			}
		}
		
		// Save out image.				
		// NEILO TODO: Generate an automatic filename.
		Img::save( "screenshot.png", pImage );

		// Free image.
		delete pImage;

		// Free image data.
		delete [] pImageData;

		// No more screenshot requested.
		ScreenshotRequested_ = BcFalse;
	}

#if PLATFORM_WINDOWS
	::SwapBuffers( WindowDC_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextGL::takeScreenshot()
{
	ScreenshotRequested_ = BcTrue;
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
		2,												// Version Number
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

	// Create a rendering context to start with.
	WindowRC_ = wglCreateContext( WindowDC_ );
	BcAssertMsg( WindowRC_ != NULL, "RsCoreImplGL: Render context is NULL!" );

	// Make current.
	wglMakeCurrent( WindowDC_, WindowRC_ );

	// Init GLEW.
	glewInit();

	
	// Attempt to create core profile.
	std::pair< BcU32, BcU32 > versions[] = 
	{
		std::pair< BcU32, BcU32 >( 4, 4 ),
		std::pair< BcU32, BcU32 >( 4, 3 ),
		std::pair< BcU32, BcU32 >( 4, 2 ),
		std::pair< BcU32, BcU32 >( 4, 1 ),
		std::pair< BcU32, BcU32 >( 4, 0 ),
		std::pair< BcU32, BcU32 >( 3, 3 ),
		std::pair< BcU32, BcU32 >( 3, 2 ),
	};

	HGLRC ParentContext = pParent_ != NULL ? pParent_->WindowRC_ : NULL;
	bool success = false;
	for( auto version : versions )
	{
		if( createProfile( version.first, version.second, BcTrue, ParentContext ) )
		{
			BcPrintf( "RsContextGL: Created OpenGL %u.%u Core Profile.\n", version.first, version.second );
			break;
		}
	}

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

	//
	RsGLCatchError;

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

//////////////////////////////////////////////////////////////////////////
// createProfile
bool RsContextGL::createProfile( BcU32 Maj, BcU32 Min, BcBool IsCore, HGLRC ParentContext )
{
	int ContextAttribs[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, Maj,
		WGL_CONTEXT_MINOR_VERSION_ARB, Min,
		WGL_CONTEXT_PROFILE_MASK_ARB, IsCore ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		NULL
	};

	HGLRC CoreProfile = wglCreateContextAttribsARB( WindowDC_, ParentContext, ContextAttribs );
	if( CoreProfile != NULL )
	{
		// release old context.
		wglMakeCurrent( WindowDC_, NULL );
		wglDeleteContext( WindowRC_ );


		// make new current.
		wglMakeCurrent( WindowDC_, CoreProfile );

		// Assign new.
		WindowRC_ = CoreProfile;

		return true;
	}

	return false;
}

#endif
