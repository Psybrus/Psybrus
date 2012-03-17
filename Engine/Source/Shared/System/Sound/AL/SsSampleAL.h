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

#include <ivorbisfile.h>

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


	// NEILO HACK: Do ogg decoding in here. Binning whole system in favour of Wwise anyway.
private:
	//
	BcBool				openOgg();
	void				closeOgg();
	BcU32				readOgg( BcU8* pBuffer, BcU32 nBytes );		
	BcU32				decode( BcU8* pBuffer, BcU32 Bytes );

	// Ogg callbacks
	static size_t		read_func( void *ptr, size_t size, size_t nmemb, void *datasource );
	static int			seek_func( void *datasource, ogg_int64_t offset, int whence );
	static int			close_func( void *datasource );
	static long			tell_func( void *datasource );
	
	vorbis_info*		pInfo_;
	OggVorbis_File		OggFile_;
	BcU32				Channels_;
	
	// Callback stuff.
	ov_callbacks		OurFileCallbacks_;
	ogg_int64_t			StreamPos_;
	BcBool				HasEnded_;
};


#endif
