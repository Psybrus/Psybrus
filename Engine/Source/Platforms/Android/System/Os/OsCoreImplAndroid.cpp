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

#include "System/Os/SDL/OsCoreImplAndroid.h"
#include "System/Os/SDL/OsClientSDL.h"

#include "System/SysKernel.h"

#include <android.h>

extern struct android_app* GState_;

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplAndroid );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplAndroid::OsCoreImplAndroid()
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

}
