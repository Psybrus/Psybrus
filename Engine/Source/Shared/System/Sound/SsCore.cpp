/**************************************************************************
 *
 * File:		SsCore.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Sound/SsCore.h"
#include "System/Sound/SsBus.h"
#include "System/Sound/SsChannel.h"
#include "System/Sound/SsFilter.h"
#include "System/Sound/SsSource.h"

//////////////////////////////////////////////////////////////////////////
// Worker mask.
size_t SsCore::JOB_QUEUE_ID = (size_t)-1;


//////////////////////////////////////////////////////////////////////////
// Reflection.
REFLECTION_DEFINE_DERIVED( SsCore );

void SsCore::StaticRegisterClass()
{
	ReRegisterAbstractClass< SsCore, Super >();

	ReEnumConstant* SsChannelStateEnumConstants[] =
	{
		new ReEnumConstant( "IDLE", (BcU32)SsChannelState::IDLE ),
		new ReEnumConstant( "PREPARED", (BcU32)SsChannelState::PREPARED ),
		new ReEnumConstant( "PLAYING", (BcU32)SsChannelState::PLAYING ),
		new ReEnumConstant( "PAUSED", (BcU32)SsChannelState::PAUSED ),
		new ReEnumConstant( "STOPPED", (BcU32)SsChannelState::STOPPED ),
	};
	ReRegisterEnum< SsChannelState >( SsChannelStateEnumConstants );

}

//////////////////////////////////////////////////////////////////////////
// setChannelParams
void SsCore::setChannelParams( class SsChannel* Channel, const class SsChannelParams& Params )
{
	Channel->setParams( Params );
}
