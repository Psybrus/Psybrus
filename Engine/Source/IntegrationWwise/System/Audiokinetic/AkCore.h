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

#ifndef __AkCore_H__
#define __AkCore_H__

#include "System/Content/CsTypes.h"
#include "System/Content/CsResource.h"

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

#include "AkIncludes.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class AkCore;

//////////////////////////////////////////////////////////////////////////
// SsCore
class AkCore:
	public SysSystem,
	public BcGlobal< AkCore >
{
public:
	AkCore();
	virtual ~AkCore();
	
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

	class CAkFilePackageLowLevelIOBlocking* pLowLevelIO_;
};

#endif

