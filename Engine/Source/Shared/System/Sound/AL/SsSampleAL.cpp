/**************************************************************************
 *
 * File:		SsSampleAL.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "SsSampleAL.h"

#include "BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SsSampleAL::SsSampleAL( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize )
{
	SampleRate_ = SampleRate;
	pData_ = pData;
	DataSize_ = DataSize;

	if( Channels == 1 )
	{
		Format_ = AL_FORMAT_MONO16;
	}
	else if( Channels == 2 )
	{
		Format_ = AL_FORMAT_STEREO16;
	}

	IsLooping_ = Looping;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SsSampleAL::~SsSampleAL()
{

}

//////////////////////////////////////////////////////////////////////////
// isLooping
BcBool SsSampleAL::isLooping() const
{
	return IsLooping_;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void SsSampleAL::create()
{
	ALuint Handle;
	
	// Create buffer.
	alBreakOnError();
	alGenBuffers( 1, &Handle );
	alBreakOnError();

	// Set handle.
	setHandle( Handle );

	// Update.
	update();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
#include "BcStream.h"
void SsSampleAL::update()
{
	ALuint Handle = getHandle< ALuint >();

	BcU8* pData = (BcU8*)pData_;

	if( pData != NULL )
	{
		// HACK./
		if( pData[ 0 ] == 'O' &&
			pData[ 1 ] == 'g' &&
			pData[ 2 ] == 'g' &&
			pData[ 3 ] == 'S' )
		{
			BcStream DecodedAudioData;

			openOgg();

			BcU32 Decoded = 0;
			BcU8 Data[ 1024 * 32 ];
			do
			{
				Decoded = decode( Data, sizeof( Data ) );
				DecodedAudioData.push( Data, Decoded );
			}
			while( Decoded > 0 );
		
			closeOgg();
			
			alBufferData( Handle, Format_, DecodedAudioData.pData(), DecodedAudioData.dataSize(), SampleRate_ );
		}
		else
		{
			alBufferData( Handle, Format_, pData_, DataSize_, SampleRate_ );
		}
	}

	alBreakOnError();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void SsSampleAL::destroy()
{
	ALuint Handle = getHandle< ALuint >();

	// Destroy AL buffer.
	alDeleteBuffers( 1, &Handle );
	
	setHandle< ALuint >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// decode
BcU32 SsSampleAL::decode( BcU8* pBuffer, BcU32 Bytes )
{
	// If the stream hasn't ended 
	if( HasEnded_ == BcFalse )
	{
		// TODO: Implement looping!
		BcU32 BytesRead = readOgg( pBuffer, Bytes );
		
		return BytesRead;
	}

	return ( HasEnded_ == BcFalse ) ? Bytes : 0;
}

////////////////////////////////////////////////////////////////////////////////
// open
BcBool SsSampleAL::openOgg()
{
	BcBool RetVal = BcFalse;

	StreamPos_ = 0;
	HasEnded_ = BcFalse;
	
	// Setup callbacks
	OurFileCallbacks_.close_func = &SsSampleAL::close_func;
	OurFileCallbacks_.tell_func = &SsSampleAL::tell_func;
	OurFileCallbacks_.read_func = &SsSampleAL::read_func;
	OurFileCallbacks_.seek_func = &SsSampleAL::seek_func;

	// Pass file handle to Ogg library.
	BcS32 Result = ov_open_callbacks( this, &OggFile_, NULL, 0, OurFileCallbacks_ );

	if ( !(Result < 0) )
	{
		pInfo_ = ov_info( &OggFile_, -1 );
		
		Channels_ = pInfo_->channels;
		SampleRate_ = pInfo_->rate;
		RetVal = BcTrue;
	}
	else
	{
		RetVal = BcFalse;
	}
	
	return RetVal;
}

////////////////////////////////////////////////////////////////////////////////
// close
void SsSampleAL::closeOgg()
{
	// Close file.
	ov_clear( &OggFile_ );
}

////////////////////////////////////////////////////////////////////////////////
// read
BcU32 SsSampleAL::readOgg( BcU8* pBuffer, BcU32 nBytes )
{
	long BytesRead = 0;
	int BStream = 0;
	char* pWritePos = (char*)pBuffer;

	// Fill the buffer
	while( BytesRead < (BcS32)nBytes )
	{
		long BytesLeft = ( nBytes - BytesRead );
		long Bytes = 0;
		long BytesToRead = BytesLeft;
		
		Bytes = ov_read( &OggFile_,
		                 pWritePos,
		                 BytesToRead,
		                 &BStream );

		// Quick error check
		if ( Bytes == OV_HOLE || Bytes == OV_EBADLINK )
		{
			break;
		}
		
		// Inc
		BytesRead += Bytes;
		pWritePos += Bytes;
		
		// Check
		if ( Bytes == 0 )
		{
			break;
		}
	}

	return BytesRead;
}

////////////////////////////////////////////////////////////////////////////////
// 
size_t SsSampleAL::read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	SsSampleAL* pSoundStream = reinterpret_cast< SsSampleAL* >( datasource );
	ogg_int64_t OldStreamPos = pSoundStream->StreamPos_;
	ogg_int64_t BytesToCopy = BcMin( (ogg_int64_t)pSoundStream->DataSize_ - pSoundStream->StreamPos_, (ogg_int64_t)size );
	BytesToCopy = BcMax( BytesToCopy, (ogg_int64_t)0 );

	BcU8* pData = (BcU8*)pSoundStream->pData_;
	BcMemCopy( ptr, &pData[ OldStreamPos ], BytesToCopy );

	// Advance stream pos.
	pSoundStream->StreamPos_ += BytesToCopy;
	
	return ( pSoundStream->StreamPos_ - OldStreamPos );
}

////////////////////////////////////////////////////////////////////////////////
// 
int SsSampleAL::seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	SsSampleAL* pSoundStream = reinterpret_cast< SsSampleAL* >( datasource );
	switch( whence )
	{
	case 0:		// SEEK_SET: Absolute.
		pSoundStream->StreamPos_ = offset;
		break;

	case 1:		// SEEK_CUR: Relative from current.
		pSoundStream->StreamPos_ = pSoundStream->StreamPos_ + offset;
		break;

	case 2:		// SEEK_END: Relative from end.
		pSoundStream->StreamPos_ = (ogg_int64_t)pSoundStream->DataSize_ - offset;
		break;
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
int SsSampleAL::close_func(void *datasource)
{
	SsSampleAL* pSoundStream = reinterpret_cast< SsSampleAL* >( datasource );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 
long SsSampleAL::tell_func(void *datasource)
{
	SsSampleAL* pSoundStream = reinterpret_cast< SsSampleAL* >( datasource );
	return (long)pSoundStream->StreamPos_;
}
