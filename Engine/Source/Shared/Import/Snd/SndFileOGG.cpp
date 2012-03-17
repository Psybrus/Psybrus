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

#include "SndFileOGG.h"

#include "BcString.h"

size_t read_func( void* ptr, size_t size, size_t nmemb, void* datasource )
{
	return fread( ptr, size, nmemb, (FILE*)datasource );
}

int seek_func( void* datasource, ogg_int64_t offset, int whence )
{
	return fseek( (FILE*)datasource, (long)offset, whence );
}

int close_func( void* datasource )
{
	return fclose( (FILE*)datasource );
}

long tell_func( void* datasource )
{
	return ftell( (FILE*)datasource );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
SndFileOGG::SndFileOGG():
	pInfo_( NULL )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SndFileOGG::~SndFileOGG()
{
	delete [] pFileData_;
	pFileData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcU32 SndFileOGG::getSampleRate() const
{
	return SampleRate_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcU32 SndFileOGG::getNumChannels() const
{
	return Channels_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
void* SndFileOGG::getData() const
{
	return pFileData_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcU32 SndFileOGG::getDataSize() const
{
	return FileDataSize_;
}

//////////////////////////////////////////////////////////////////////////
// LoadCallback
BcBool SndFileOGG::load( const BcChar* pFileName )
{
	// Open file.
	FILE* pFile = fopen( pFileName, "rb" );
	
	if( pFile != NULL )
	{
		OurFileCallbacks_.close_func = ::close_func;
		OurFileCallbacks_.seek_func = ::seek_func;
		OurFileCallbacks_.read_func = ::read_func;
		OurFileCallbacks_.tell_func = ::tell_func;
		int Result = ov_open_callbacks( pFile, &OggFile_, NULL, 0, OurFileCallbacks_ );
		
		if( !(Result < 0 ) )
		{
			pInfo_ = ov_info( &OggFile_, -1 );
			Channels_ = pInfo_->channels;
			SampleRate_ = pInfo_->rate;

			// Read it all in. We'll need it later.
			BcFile AllTheOgg;
			if( AllTheOgg.open( pFileName ) )
			{
				FileDataSize_ = AllTheOgg.size();
				pFileData_ = new BcU8[ FileDataSize_ ];
				AllTheOgg.read( pFileData_, FileDataSize_ );
				AllTheOgg.close();
			}

			ov_clear( &OggFile_ );
			return BcTrue;
		}
	}
	return BcFalse;
}
