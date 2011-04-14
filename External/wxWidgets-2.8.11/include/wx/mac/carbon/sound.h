/////////////////////////////////////////////////////////////////////////////
// Name:        sound.h
// Purpose:     wxSound class (loads and plays short Windows .wav files).
//              Optional on non-Windows platforms.
// Author:      Ryan Norton, Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: sound.h 62181 2009-09-28 08:12:24Z JS $
// Copyright:   (c) Ryan Norton, Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SOUND_H_
#define _WX_SOUND_H_

#if wxUSE_SOUND

#include "wx/object.h"

class WXDLLEXPORT wxSound : public wxSoundBase
{
public:
  wxSound();
  wxSound(const wxString& fileName, bool isResource = false);
  wxSound(int size, const wxByte* data);
  virtual ~wxSound();

public:
  bool  Create(const wxString& fileName, bool isResource = false);
  bool  IsOk() const { return !m_sndname.IsEmpty() || m_hSnd; }
  static void  Stop();
  static bool IsPlaying();

  void* GetHandle();
protected:  
  bool  DoPlay(unsigned flags) const;
public:
#if wxABI_VERSION >= 20811
  bool  Create(int size, const wxByte* data);
#endif
private:
    wxString m_sndname; //file path
    char* m_hSnd; //pointer to resource or memory location
    int m_waveLength; //size of file in memory mode
    void* m_pTimer; //timer

    enum wxSoundType
    {
        wxSound_MEMORY,
        wxSound_FILE,
        wxSound_RESOURCE,
        wxSound_NONE
    } m_type; //mode
};

#endif
#endif
    // _WX_SOUND_H_
