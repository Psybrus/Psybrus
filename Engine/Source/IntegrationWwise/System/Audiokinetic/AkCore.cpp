/**************************************************************************
 *
 * File:		AkCore.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Audiokinetic/AkCore.h"

#include "Common/AkFilePackageLowLevelIOBlocking.h"		// Low level io

#define DEMO_DEFAULT_POOL_SIZE (2*1024*1024)
#define DEMO_LENGINE_DEFAULT_POOL_SIZE (2*1024*1024)
#define __AK_OSCHAR_SNPRINTF _snwprintf

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( AkCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
AkCore::AkCore()
{
	pLowLevelIO_ = new CAkFilePackageLowLevelIOBlocking();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
AkCore::~AkCore()
{
	delete pLowLevelIO_;
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void AkCore::open()
{
	// Get default settings from Wwise.
	MemSettings_.uMaxNumPools = 20;
	AK::StreamMgr::GetDefaultSettings( StmSettings_ );
	AK::StreamMgr::GetDefaultDeviceSettings( DeviceSettings_ );
	AK::SoundEngine::GetDefaultInitSettings( InitSettings_ );
	InitSettings_.uDefaultPoolSize = DEMO_DEFAULT_POOL_SIZE;
	//InitSettings_.pfnAssertHook = INTEGRATIONDEMO_ASSERT_HOOK;
	AK::SoundEngine::GetDefaultPlatformInitSettings( PlatformInitSettings_ );
	PlatformInitSettings_.uLEngineDefaultPoolSize = DEMO_LENGINE_DEFAULT_POOL_SIZE;
	AK::MusicEngine::GetDefaultInitSettings( MusicInit_ );

	// Init
	const int ErrorBufferSize = 4096;
	AkOSChar ErrorBuffer[ ErrorBufferSize ];

    //
    // Create and initialize an instance of the default memory manager. Note
    // that you can override the default memory manager with your own. Refer
    // to the SDK documentation for more information.
    //

	AKRESULT res = AK::MemoryMgr::Init( &MemSettings_ );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("AK::MemoryMgr::Init() returned AKRESULT %d"), res );
        BcBreakpoint;
    }

	//
    // Create and initialize an instance of the default streaming manager. Note
    // that you can override the default streaming manager with your own. Refer
    // to the SDK documentation for more information.
    //

    // Customize the Stream Manager settings here.
    
    if ( !AK::StreamMgr::Create( StmSettings_ ) )
    {
		AKPLATFORM::SafeStrCpy( ErrorBuffer, AKTEXT( "AK::StreamMgr::Create() failed" ), ErrorBufferSize );
        BcBreakpoint;
    }
    
	// 
    // Create a streaming device with blocking low-level I/O handshaking.
    // Note that you can override the default low-level I/O module with your own. Refer
    // to the SDK documentation for more information.        
	//
    
    // CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
    // in the Stream Manager, and registers itself as the File Location Resolver.
	res = pLowLevelIO_->Init( DeviceSettings_ );
	if ( res != AK_Success )
	{
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("m_lowLevelIO.Init() returned AKRESULT %d"), res );
        BcBreakpoint;
    }

    //
    // Create the Sound Engine
    // Using default initialization parameters
    //
    
	res = AK::SoundEngine::Init( &InitSettings_, &PlatformInitSettings_ );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("AK::SoundEngine::Init() returned AKRESULT %d"), res );
        BcBreakpoint;
    }

    //
    // Initialize the music engine
    // Using default initialization parameters
    //

	res = AK::MusicEngine::Init( &MusicInit_ );
    if ( res != AK_Success )
    {
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("AK::MusicEngine::Init() returned AKRESULT %d"), res );
        BcBreakpoint;
    }

#if !PSY_PRODUCTION
    //
    // Initialize communications (not in release build!)
    //
	AkCommSettings CommSettings;
	AK::Comm::GetDefaultInitSettings( CommSettings );
	res = AK::Comm::Init( CommSettings );
	if ( res != AK_Success )
	{
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("AK::Comm::Init() returned AKRESULT %d. Communication between the Wwise authoring application and the game will not be possible."), res );
	}
#endif // !PSY_PRODUCTION
	
	//
	// Register plugins
	/// Note: This a convenience method for rapid prototyping. 
	/// To reduce executable code size register/link only the plug-ins required by your game 
	res = AK::SoundEngine::RegisterAllPlugins();
	if ( res != AK_Success )
	{
		__AK_OSCHAR_SNPRINTF( ErrorBuffer, ErrorBufferSize, AKTEXT("AK::SoundEngine::RegisterAllPlugins() returned AKRESULT %d."), res );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void AkCore::update()
{
	AK::SoundEngine::RenderAudio();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void AkCore::close()
{
#if !PSY_PRODUCTION
	// Terminate communications between Wwise and the game
	AK::Comm::Term();
#endif // !PSY_PRODUCTION

	// Terminate the music engine
	AK::MusicEngine::Term();

	// Terminate the sound engine
	if ( AK::SoundEngine::IsInitialized() )
	{
		AK::SoundEngine::Term();
	}

	// Terminate the streaming device and streaming manager
	// CAkFilePackageLowLevelIOBlocking::Term() destroys its associated streaming device 
	// that lives in the Stream Manager, and unregisters itself as the File Location Resolver.
    if ( AK::IAkStreamMgr::Get() )
	{
		//m_pLowLevelIO->Term();
		AK::IAkStreamMgr::Get()->Destroy();
	}

	// Terminate the Memory Manager
	if ( AK::MemoryMgr::IsInitialized() )
	{
		AK::MemoryMgr::Term();
	}
}
