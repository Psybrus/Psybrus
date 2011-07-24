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

#ifndef __SSVORBISSTREAMAL_H__
#define __SSVORBISSTREAMAL_H__


#include "exOS.h"

#ifdef EX_LINUX
# include <AL/al.h>
# include <AL/alc.h>
#endif
#ifdef EX_WIN32
# include <al.h>
# include <alc.h>
# include <efx.h>
# include <EFX-Util.h>
# include <efx-creative.h>
#endif

//#include "vorbisfile.h"

#include "exSsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class exSsCoreALInternal;

//////////////////////////////////////////////////////////////////////////
// exSsBufferAL
struct exSsBufferAL
{
	exU8*		pBuffer_;
	exU32		Size_;
	ALuint		ALBuffer_;
};

//////////////////////////////////////////////////////////////////////////
// exSsVorbisStreamAL
class exSsVorbisStreamAL
{
public:
	enum
	{
		BUFFER_COUNT = 4,
		BUFFER_SIZE = ( 1024 * 128 ),
	};

public:
	exSsVorbisStreamAL();
	~exSsVorbisStreamAL();

	exBool		open( exConstChar* FileName );
	exBool		close();

	exU32		channels() const;
	exU32		rate() const;

	exSsBufferAL* pNextBuffer();

	exConstChar* trackArtist() const;
	exConstChar* trackTitle() const;

private:
	friend class exSsCoreALInternal;

	exU32		decode( exU8* pBuffer, exU32 Bytes );

private:
	/*
	exChar				FileName_[ 256 ];
	FILE*				pFile_;
	vorbis_info*		pInfo_;
	OggVorbis_File		OggFile_;

	exChar				Artist_[ 256 ];
	exChar				Title_[ 256 ];

	exU32				CurrentBuffer_;
	exFArray< exSsBufferAL, BUFFER_COUNT >		Buffers_;
	*/
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline exU32 exSsVorbisStreamAL::channels() const
{
	return 0;//pInfo_->channels;
}

inline exU32 exSsVorbisStreamAL::rate() const
{
	return 0;//pInfo_->rate;
}

inline exConstChar* exSsVorbisStreamAL::trackArtist() const
{
	return EX_NULL;//Artist_;
}

inline exConstChar* exSsVorbisStreamAL::trackTitle() const
{
	return EX_NULL;//Title_;
}

#endif
