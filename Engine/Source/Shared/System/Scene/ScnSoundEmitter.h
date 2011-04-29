/**************************************************************************
*
* File:		ScnSoundEmitter.h
* Author:	Neil Richardson 
* Ver/Date:	28/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSoundEmitter_H__
#define __ScnSoundEmitter_H__

#include "CsResourceRef.h"
#include "ScnSound.h"

#include "SsChannel.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitterRef
typedef CsResourceRef< class ScnSoundEmitter > ScnSoundEmitterRef;

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitter
class ScnSoundEmitter:
	public CsResource,
	public SsChannelCallback
{
public:
	DECLARE_RESOURCE( CsResource, ScnSoundEmitter );
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	void								play( ScnSoundRef Sound );

protected:
	virtual void						onStarted( SsChannel* pSound );
	virtual void						onPlaying( SsChannel* pSound );
	virtual void						onEnded( SsChannel* pSound );
	
private:
	typedef std::map< SsChannel*, ScnSoundRef >	TChannelSoundMap;
	typedef TChannelSoundMap::iterator	TChannelSoundMapIterator;

	TChannelSoundMap					ChannelSoundMap_;
};


#endif


