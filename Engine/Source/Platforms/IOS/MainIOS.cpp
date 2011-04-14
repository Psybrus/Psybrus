#include "SysKernel.h"

/*
int main( int argc, char* argv[] )
{
	// Create kernel.
	SysKernel Kernel;
	
	// Set command line params.
	SysArgc_ = (BcU32)argc;
	SysArgv_ = (BcChar**)&argv[0];
	
	// Register systems for creation.
	SYS_REGISTER( &Kernel, OsCoreImplSDL );
	SYS_REGISTER( &Kernel, FsCoreImplOSX );
	SYS_REGISTER( &Kernel, CsCoreImplShared );
	SYS_REGISTER( &Kernel, RsCoreImplGL );
	
	// Pass over to user for additional configuration
	extern void PsyGameInit( SysKernel& Kernel );
	PsyGameInit( Kernel );
	
	// No need to unbind these delegates.
	return 0;
}
 */

// TEST BED STUFF...
#include "SysKernel.h"
#include "OsCore.h"
#include "CsCore.h"
#include "FsCore.h"
#include "RsCore.h"
#include "RmCore.h"

#include "CsFile.h"

#include "RsCoreImplGLES.h"

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


void onFileReady( CsFile* pFile )
{
	
}

void onFileChunk( CsFile*, CsFileChunk* pChunk, void* pData )
{	
	
}


RsProgram* gpProgram = NULL;
eEvtReturn doUpdate( EvtID ID, const SysSystemEvent& Event )
{
	/*
	 if( CsCore::pImpl()->getResource( "test.pmat", Texture ) )
	 {
	 BcPrintf( "Got texture!\n" );	
	 }
	 */
	
	//File.load( "test.ptex", CsFileReadyDelegate::bind< onFileReady >(), CsFileChunkDelegate::bind< onFileChunk >() );
	
	// No more updates please.
	//RsCore::pImpl()->unsubscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );
	
	
	// Test creating of vertex and fragment programs.
	static BcBool Done = BcFalse;
	
	if( Done == BcFalse )
	{
		Done = BcTrue;
	
		char* pVertexShaderSource = 
		"attribute vec4 aPosition;						"
		"attribute vec4 aColour;						"
		"varying vec4 colorVarying;						"
		"varying lowp vec2 uvPosition;				"
		"uniform float translate;						"
		"void main()									"
		"{												"
		"	gl_Position = aPosition;					"
		"	gl_Position.y += sin(translate) / 2.0;		"
		"	colorVarying = aColour;						"
		"	uvPosition = aPosition.xy;				"
		"}												";
		
		
		char* pFragmentShaderSource = 
		"varying lowp vec4 colorVarying;				"
		"varying lowp vec2 uvPosition;				"
		"uniform sampler2D texture;								"
		"void main()									"
		"{												"
		"	gl_FragColor = colorVarying * texture2D(texture, uvPosition);				"
		"}												";
		
	
		RsShader* pVertexShader = RsCore::pImpl()->createShader( rsST_VERTEX, rsSDT_SOURCE, pVertexShaderSource, 0 );
		RsShader* pFragmentShader = RsCore::pImpl()->createShader( rsST_FRAGMENT, rsSDT_SOURCE, pFragmentShaderSource, 0 );
		gpProgram = RsCore::pImpl()->createProgram( pVertexShader, pFragmentShader );
	}	
}

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
		
	}
};

eEvtReturn doRender( EvtID ID, const SysSystemEvent& Event )
{
	// Allocater a frame from the renderer.
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame();
	
	// Setup view.
	RsViewport Viewport( 0, 0, 640, 480 );
	BcMat4d View;
	
	static BcReal Ticker = 0.0f;
	Ticker += 0.01f;
	View.rotation( BcVec3d( 0.0f, 0.0f, Ticker ) );
	
	BcReal W = 640.0f * 0.5f;
	BcReal H = 480.0f * 0.5f;
	
	Viewport.orthoProj( -W, W, H, -H, -1.0f, 1.0f );
	Viewport.view( View );
	
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );
	
	pFrame->addRenderNode( pFrame->newObject< TestNode >( RsRenderSort( 0 ) ) );
	
	// Queue frame up to be rendered.
	RsCore::pImpl()->queueFrame( pFrame );	
	
	return evtRET_PASS;
}

eEvtReturn doRemoteOpened( EvtID ID, const SysSystemEvent& Event )
{
	// Connect to engine server.
	RmCore::pImpl()->connect( "192.168.1.100"/*"neilogd.dyndns.org"*/ );
}

void PsyGameInit( SysKernel& Kernel )
{
	//
	extern void BcAtomic_UnitTest();
	BcAtomic_UnitTest();
	
	// Start up systems.
//	Kernel.startSystem( "OsCoreImplSDL" );
//	Kernel.startSystem( "FsCoreImplOSX" );
//	Kernel.startSystem( "CsCoreImplShared" );
	Kernel.startSystem( "RmCore" );
	Kernel.startSystem( "CsCoreClient" );
	Kernel.startSystem( "RsCoreImplGLES" );
	
	// Bind delegates
	DelegateQuit = OsEventCore::Delegate::bind< doQuit >();
	DelegateRender = SysSystemEvent::Delegate::bind< doRender >();	
	DelegateUpdate = SysSystemEvent::Delegate::bind< doUpdate >();	
	DelegateRemoteOpened = SysSystemEvent::Delegate::bind< doRemoteOpened >();	
	
	// Hook engine events to begin processing.
	//OsCore::pImpl()->subscribe( osEVT_CORE_QUIT,			DelegateQuit );			// Quit command to close systems.
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,	DelegateRemoteOpened );			// Quit command to close systems.
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );		// Hook pre render for testing render.
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateRender );		// Hook pre render for testing render.
	
	// Run the kernel.
	//Kernel.run();
}


