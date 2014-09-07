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

//////////////////////////////////////////////////////////////////////////
// Worker mask.
BcU32 SsCore::JOB_QUEUE_ID = BcErrorCode;


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
