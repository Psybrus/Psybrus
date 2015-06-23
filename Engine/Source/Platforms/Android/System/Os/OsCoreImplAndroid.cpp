/**************************************************************************
*
* File:		OsCoreImplAndroid.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Os/OsCoreImplAndroid.h"
#include "System/Os/OsClientAndroid.h"

#include "System/SysKernel.h"

#include <android_native_app_glue.h>

#include <android/input.h>
#include <android/keycodes.h>

extern struct android_app* GState_;

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplAndroid );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplAndroid::OsCoreImplAndroid():
	MainThreadLooper_( nullptr )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplAndroid::~OsCoreImplAndroid()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplAndroid::open()
{
	MainThreadLooper_ = ALooper_forThread();
	ALooper_acquire( MainThreadLooper_ );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplAndroid::update()
{
	OsCore::update();

	int EventId = 0;
	int NoofEvents = 0;
	struct android_poll_source* PollSource = nullptr;
	
	while ( ( EventId = ALooper_pollAll( 0, nullptr, &NoofEvents, (void**)&PollSource) ) >= 0 )
	{
		// Process this event.
		if( PollSource != nullptr )
		{
			PollSource->process( GState_, PollSource );
		}

		// Check if we are exiting.
		if( GState_->destroyRequested != 0 )
		{
			PSY_LOG( "Engine thread destroy requested!" );
			SysKernel::pImpl()->stop();
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplAndroid::close()
{
	ALooper_release( MainThreadLooper_ );
	MainThreadLooper_ = nullptr;
}
