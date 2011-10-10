// --------------------------------------------------------------------------
// Name: sndcodec.cpp
// Purpose:
// Date: 08/11/1999
// Author: Guilhem Lavaux <lavaux@easynet.fr> (C) 1999
// CVSID: $Id: sndcodec.cpp 35650 2005-09-23 12:56:45Z MR $
// wxWindows licence
// --------------------------------------------------------------------------

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/defs.h"
#endif

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "wx/mmedia/sndbase.h"
#include "wx/mmedia/sndcodec.h"

wxSoundStreamCodec::wxSoundStreamCodec(wxSoundStream& snd_io)
 : m_sndio(&snd_io)
{
}

wxSoundStreamCodec::~wxSoundStreamCodec()
{
}

bool wxSoundStreamCodec::StartProduction(int evt)
{
  return m_sndio->StartProduction(evt);
}

bool wxSoundStreamCodec::StopProduction()
{
  return m_sndio->StopProduction();
}

wxUint32 wxSoundStreamCodec::GetBestSize() const
{
  return m_sndio->GetBestSize();
}
