#include "SysKernel.h"

#include <SDL.h>

int main( int argc, char* argv[] )
{
	// Create kernel.
	SysKernel Kernel;
	
	// Set command line params.
	SysArgc_ = (BcU32)argc;
	SysArgv_ = (BcChar**)&argv[0];
	
	// Register systems for creation.
	SYS_REGISTER( &Kernel, RmCore );
	SYS_REGISTER( &Kernel, OsCoreImplSDL );
	SYS_REGISTER( &Kernel, FsCoreImplOSX );
	SYS_REGISTER( &Kernel, CsCoreClient );
	SYS_REGISTER( &Kernel, RsCoreImplGL );
	
	// Pass over to user for additional configuration
	extern void PsyGameInit( SysKernel& Kernel );
	PsyGameInit( Kernel );
	
	// No need to unbind these delegates.
	return 0;
}

// TEST BED STUFF...
#include "SysKernel.h"
#include "OsCore.h"
#include "CsCore.h"
#include "FsCore.h"
#include "RsCore.h"
#include "RmCore.h"

#include "CsFile.h"

#include "RsCoreImplGL.h"

#include "ScnTexture.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnCanvas.h"
#include "ScnShader.h"

OsEventCore::Delegate DelegateQuit;
SysSystemEvent::Delegate DelegateRender;
SysSystemEvent::Delegate DelegateUpdate;
SysSystemEvent::Delegate DelegateRemoteOpened;

eEvtReturn doQuit( EvtID ID, const OsEventCore& Event )
{
	// Stop all systems.
	OsCore::pImpl()->pKernel()->stop();
	
	//
	return evtRET_PASS;
}

ScnTextureRef Texture;
ScnModelRef Model;
ScnCanvasRef Canvas;
ScnShaderRef VertexShaderRef;
ScnShaderRef FragmentShaderRef;

eEvtReturn doUpdate( EvtID ID, const SysSystemEvent& Event )
{
	if( Canvas.isValid() == BcFalse )
	{
		CsCore::pImpl()->registerResource< ScnTexture >();
		CsCore::pImpl()->registerResource< ScnMaterial >();
		CsCore::pImpl()->registerResource< ScnModel >();
		CsCore::pImpl()->registerResource< ScnCanvas >();
		CsCore::pImpl()->registerResource< ScnShader >();
		
		CsCore::pImpl()->createResource( "test", Canvas, 1024 );
		
		//CsCore::pImpl()->requestResource( "testv", VertexShaderRef );
		//CsCore::pImpl()->requestResource( "testf", FragmentShaderRef );
		
	}
	
	// No more updates please.
	//RsCore::pImpl()->unsubscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );

	return evtRET_PASS;
}

BcChar* VertexShader = 
"attribute vec4 aPosition;						"
"attribute vec4 aColour;						"
"varying vec4 vColour;							"
"void main()									"
"{												"
"	gl_Position = aPosition;					"
"	vColour = aColour;							"
"}												";


BcChar* FragmentShader =
"varying vec4 vColour;							"
"void main()									"
"{												"
"	gl_FragColor = vColour;						"
"}												";


RsIndexBuffer* pIndexBuffer = NULL;
RsVertexBuffer* pVertexBuffer = NULL;
RsPrimitive* pPrimitive = NULL;
RsShader* pVertexShader = NULL;
RsShader* pFragmentShader = NULL;
RsProgram* pProgram = NULL;

// Test node.
class TestNode: public RsRenderNode
{
public:
	TestNode( RsRenderSort Sort )
	{
		Sort_ = Sort;	
	}
	
	void render()
	{
		static BcReal Tick = 0.0f;
		Tick += 0.001f;
		
		pProgram->bind();
		
		/*
		glBegin( GL_LINE_STRIP );
		glVertex3f( -0.25f, -0.25f, 0.1f );
		glVertex3f(  0.25f, -0.25f, 0.1f );
		glVertex3f(  0.25f,  0.25f, 0.1f );
		glVertex3f( -0.25f,  0.25f, 0.1f );
		glVertex3f( -0.25f, -0.25f, 0.1f );
		glEnd();
		 */
	}
};

struct TVertex
{
	BcF32 X, Y, Z;
};
BcU16 Indices[] = { 0, 1, 2, 3, 0 };
TVertex Vertices[] =
{
	{ -0.25f, -0.25f, 0.1f },
	{  0.25f, -0.25f, 0.1f },
	{  0.25f,  0.25f, 0.1f },
	{ -0.25f,  0.25f, 0.1f },
	{ -0.25f, -0.25f, 0.1f },
};

eEvtReturn doRender( EvtID ID, const SysSystemEvent& Event )
{
	// Allocater a frame from the renderer.
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame();
	// Create hacking assets.
	if( pPrimitive == NULL )
	{

		pIndexBuffer = RsCore::pImpl()->createIndexBuffer( 5, Indices );
		pVertexBuffer = RsCore::pImpl()->createVertexBuffer( rsVDF_POSITION_XYZ, 5, Vertices );
		pPrimitive = RsCore::pImpl()->createPrimitive( pVertexBuffer, NULL );
	
		pVertexShader = RsCore::pImpl()->createShader( rsST_VERTEX, rsSDT_SOURCE, VertexShader, BcStrLength( VertexShader ) );
		pFragmentShader = RsCore::pImpl()->createShader( rsST_FRAGMENT, rsSDT_SOURCE, FragmentShader, BcStrLength( VertexShader ) );
		pProgram = RsCore::pImpl()->createProgram( pVertexShader, pFragmentShader );
	}
	
	// Setup view.
	RsViewport Viewport( 0, 0, 640, 480 );
	BcMat4d View;
	
	static BcReal Ticker = 0.0f;
	Ticker += 0.001f;
	View.rotation( BcVec3d( 0.0f, 0.0f, Ticker ) );
	
	BcReal W = 1280.0f * 0.5f;
	BcReal H = 720.0f * 0.5f;
	
	Viewport.orthoProj( -W, W, H, -H, -1.0f, 1.0f );
	Viewport.view( View );
	
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );
	
	pFrame->addRenderNode( pFrame->newObject< TestNode >( RsRenderSort( 0 ) ) );
	
	// Canvas test.
	if( Canvas.isReady() )
	{
		BcMat4d Transform;
		
		Transform.rotation( BcVec3d( 0.0f, 0.0f, Ticker ) );
		Transform.translation( BcVec3d( 64.0f, 64.0f, 0.0f ) );
		Canvas->clear();
		
		Canvas->pushMatrix( Viewport.projection() );
		
		Canvas->pushMatrix( Transform );		
		Canvas->drawBox( BcVec2d( -32.0f, -32.0f ), BcVec2d( 32.0f, 32.0f ), RsColour::BLUE, 0 );
		Canvas->pushMatrix( Transform );		
		Canvas->drawBox( BcVec2d( -32.0f, -32.0f ), BcVec2d( 32.0f, 32.0f ), RsColour::GREEN, 0 );

		Canvas->popMatrix();
		Canvas->popMatrix();
		Canvas->drawLine( BcVec2d( -W, -H ), BcVec2d( W, H ), RsColour::RED, 1 );
		
		Canvas->render( pFrame, RsRenderSort( 0 ) );
	}
	
	// Queue frame up to be rendered.
	RsCore::pImpl()->queueFrame( pFrame );	
	
	return evtRET_PASS;
}

eEvtReturn doRemoteOpened( EvtID, const SysSystemEvent& Event )
{
	//RmCore::pImpl()->connect( "localhost" );
}

void PsyGameInit( SysKernel& Kernel )
{
	for( BcU32 i = 0; i < 32; ++i )
	{
		BcPrintf( "Bits in %x: %i\n", i, BcBitsSet( i ) );
	}
	
	//
	extern void BcAtomic_UnitTest();
	BcAtomic_UnitTest();
	
	// Start up systems.
	Kernel.startSystem( "RmCore" );
	Kernel.startSystem( "OsCoreImplSDL" );
	Kernel.startSystem( "FsCoreImplOSX" );
	Kernel.startSystem( "CsCoreClient" );
	Kernel.startSystem( "RsCoreImplGL" );
	
	// Bind delegates
	DelegateQuit = OsEventCore::Delegate::bind< doQuit >();
	DelegateRender = SysSystemEvent::Delegate::bind< doRender >();	
	DelegateUpdate = SysSystemEvent::Delegate::bind< doUpdate >();	
	DelegateRemoteOpened = SysSystemEvent::Delegate::bind< doRemoteOpened >();	
	
	// Hook engine events to begin processing.
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT,			DelegateQuit );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateRender );
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,	DelegateRemoteOpened );	
	
	// Run the kernel.
	Kernel.run();
}


