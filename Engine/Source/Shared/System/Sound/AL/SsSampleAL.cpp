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
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SsSampleAL::~SsSampleAL()
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void SsSampleAL::create()
{
	ALuint Handle;
	
	// Create buffer.
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
void SsSampleAL::update()
{
	ALuint Handle = getHandle< ALuint >();

	alBufferData( Handle, Format_, pData_, DataSize_, SampleRate_ );
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
