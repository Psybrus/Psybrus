/**************************************************************************
*
* File:		SndFileOGG.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SndFileOGG_H__
#define __SndFileOGG_H__

#include "BcFile.h"
#include "BcDebug.h"

#include "Snd.h"

#include <ivorbisfile.h>

//////////////////////////////////////////////////////////////////////////
// SndFileOGG
class SndFileOGG:
	public SndSound
{
private:
	BcU8*				pFileData_;
	BcU32				FileDataSize_;

	BcU32				Channels_;
	BcU32				SampleRate_;
	
	vorbis_info*		pInfo_;
	OggVorbis_File		OggFile_;
	ov_callbacks		OurFileCallbacks_;
	BcU32				StreamPos_;
	
public:
	SndFileOGG();
	virtual ~SndFileOGG();
	
	BcBool				isOgg() const{return BcTrue;}
	BcU32				getSampleRate() const;
	BcU32				getNumChannels() const;
	void*				getData() const;
	BcU32				getDataSize() const;
	
	BcBool				load( const BcChar* pFileName );
};

#endif
