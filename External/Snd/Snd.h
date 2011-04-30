/**************************************************************************
*
* File:		Snd.h
* Author:	Neil Richardson 
* Ver/Date:	29/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __Snd_H__
#define __Snd_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// SndSound
class SndSound
{
public:
	virtual ~SndSound(){};
	
	virtual BcU32				getSampleRate() const = 0;
	virtual BcU32				getNumChannels() const = 0;
	virtual void*				getData() const = 0;
	virtual BcU32				getDataSize() const = 0;
};

//////////////////////////////////////////////////////////////////////////
// Snd
class Snd
{
public:
	static SndSound* load( const BcChar* pFileName );
	static SndSound* loadWav( const BcChar* pFileName );
};

#endif


