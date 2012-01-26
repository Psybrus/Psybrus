/**************************************************************************
 *
 * File:		SsSampleAL.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSSAMPLEAL_H__
#define __SSSAMPLEAL_H__

#include "SsSample.h"
#include "SsAL.h"

//////////////////////////////////////////////////////////////////////////
// SsSampleAL
class SsSampleAL:
	public SsSample
{
private:
	BcU32		SampleRate_;
	ALuint		Format_;
	BcBool		IsLooping_;

public:
	SsSampleAL( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize );
	virtual ~SsSampleAL();

	BcBool isLooping() const;
	
protected:
	virtual void		create();
	virtual void		update();
	virtual void		destroy();

};


#endif
