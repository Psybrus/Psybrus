/**************************************************************************
 *
 * File:		AksCore.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __AKSCORE_H__
#define __AKSCORE_H__

#include "System/Sound/SsChannel.h"
#include "System/Sound/SsSample.h"
#include "System/Sound/SsEnvironment.h"

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

#include "AkIncludes.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class AksCore;

//////////////////////////////////////////////////////////////////////////
// SsCore
class AksCore:
	public SysSystem,
	public BcGlobal< AksCore >
{
public:
	AksCore();
	virtual ~AksCore();
	
	virtual void				open();
	virtual void				update();
	virtual void				close();


private:
	AkMemSettings			MemSettings_;
	AkStreamMgrSettings		StmSettings_;
	AkDeviceSettings		DeviceSettings_;
	AkInitSettings			InitSettings_;
	AkPlatformInitSettings	PlatformInitSettings_;
	AkMusicSettings			MusicInit_;
};

#endif

