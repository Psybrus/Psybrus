/**************************************************************************
*
* File:		Sound/ScnSoundFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSOUNDFILEDATA__
#define __SCNSOUNDFILEDATA__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundHeader
struct ScnSoundHeader
{
	BcU32 SampleRate_;
	BcU32 Channels_;
	BcBool Looping_;
	BcBool IsOgg_;
};

#endif
