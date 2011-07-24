// ============================================================================
//
// Copyright © 2010 Alice Blunt & Neil Richardson.
//
// This file is part of ExcaliburEx.
//
// ExcaliburEx is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ExcaliburEx is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ExcaliburEx.  If not, see <http://www.gnu.org/licenses/>.
//
// ============================================================================

#include "exSsVorbisStreamAL.h"

#define AL_BREAKONERROR \
	{\
	ALenum Error = alGetError(); \
	if( Error != 0 )\
		{\
		BcPrintf( "OpenAL Error: %x\n", Error );\
		BcBreakpoint;\
		}\
	}

//////////////////////////////////////////////////////////////////////////
// Ctor
exSsVorbisStreamAL::exSsVorbisStreamAL()
{
	/*
	// Create buffers.
	for( exU32 i = 0; i < Buffers_.Size(); ++i )
	{
		Buffers_[ i ].pBuffer_ = new exU8[ BUFFER_SIZE ];
		Buffers_[ i ].Size_ = 0;

		// AL Buffer.
		alGenBuffers( 1, &Buffers_[ i ].ALBuffer_ );
	}

	CurrentBuffer_ = 0;

	Artist_[0] = '\0';
	Title_[0] = '\0';
	*/
}

//////////////////////////////////////////////////////////////////////////
// Dtor
exSsVorbisStreamAL::~exSsVorbisStreamAL()
{
	/*
	// Delete buffers.
	for( exU32 i = 0; i < Buffers_.Size(); ++i )
	{
		Buffers_[ i ].Size_ = 0;
		delete [] Buffers_[ i ].pBuffer_;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
// open
exBool exSsVorbisStreamAL::open( exConstChar* FileName )
{
	/*
	// Open file
	if( FileName != EX_NULL )
	{
		exString::StrCpy( FileName_, FileName );
	}

	pFile_ = fopen( FileName_, "rb" );

	// If valid open with ogg vorbis libs.
	if( pFile_ != EX_NULL )
	{
		// Open.
		ov_open( pFile_, &OggFile_, EX_NULL, 0 );

		// Get info.
		pInfo_ = ov_info( &OggFile_, -1 );

		// Extract some info.
		for( exU32 i = 0; i < OggFile_.vc->comments; ++i )
		{
			exChar Buffer[ 1024 ];
			exU32 BufferPos = 0;
			char* pComment = OggFile_.vc->user_comments[ i ];
			int Length = OggFile_.vc->comment_lengths[ i ];

			for( exU32 j = 0; j < Length; ++j )
			{
				Buffer[ j ] = pComment[ j ];
			}
			Buffer[ Length ] = '\0';

			if( exString::StrStr( Buffer, "ARTIST=" ) != EX_NULL )
			{
				exString::StrCpy( Artist_, exString::StrStr( Buffer, "=" ) + 1 );
			}
			else if( exString::StrStr( Buffer, "TITLE=" ) != EX_NULL )
			{
				exString::StrCpy( Title_, exString::StrStr( Buffer, "=" ) + 1 );
			}
		}

		//
		return exTrue;
	}

	*/
	return exFalse;
}

//////////////////////////////////////////////////////////////////////////
// close
exBool exSsVorbisStreamAL::close()
{
	/*
	if( pFile_ != EX_NULL )
	{
		ov_clear( &OggFile_ );

		fclose( pFile_ );
		pFile_ = EX_NULL;

		return exTrue;
	}

	*/
	return exFalse;
}

//////////////////////////////////////////////////////////////////////////
// pNextBuffer
exSsBufferAL* exSsVorbisStreamAL::pNextBuffer()
{
	/*
	exSsBufferAL* pBuffer = &Buffers_[ CurrentBuffer_ ];

	// Advance buffer.
	CurrentBuffer_ = ( CurrentBuffer_ + 1 ) % Buffers_.Size();

	// Decode to buffer.
	pBuffer->Size_ = decode( pBuffer->pBuffer_, BUFFER_SIZE );

	// Buffer to AL.
	if( pBuffer->Size_ > 0 )
	{
		alBufferData( pBuffer->ALBuffer_, AL_FORMAT_STEREO16, pBuffer->pBuffer_, pBuffer->Size_, rate() );

		// We get an error sometimes here...tis a bit odd :-S
		if ( alGetError() )
		{
			pBuffer->Size_ = 0;
		}
	}

	// Return it.
	return pBuffer;	
	*/
	return EX_NULL;
}

//////////////////////////////////////////////////////////////////////////
// decode
exU32 exSsVorbisStreamAL::decode( exU8* pBuffer, exU32 Bytes )
{
	/*
	exU32 BytesRead = 0;
	exS32 BitStream;
	exS32 OggRetVal;

	do 
	{
		// Read in data.
		OggRetVal = ov_read( &OggFile_, (char*)&pBuffer[ BytesRead ], Bytes - BytesRead, 0, 2, 1, &BitStream );

		// Increment buffer pointer.
		if( OggRetVal > 0 )
		{
			BytesRead += OggRetVal;
		}
	}
	while ( ( OggRetVal > 0 ) && ( BytesRead < Bytes )  );

	return BytesRead;
	*/
	return 0;
}
