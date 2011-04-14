// --------------------------------------------------------------------------
// Name: sndpcm.h
// Purpose:
// Date: 08/11/1999
// Author: Guilhem Lavaux <lavaux@easynet.fr> (C) 1999
// CVSID: $Id: sndpcm.h 35650 2005-09-23 12:56:45Z MR $
// License:    wxWindows license
// --------------------------------------------------------------------------
#ifndef _WX_SNDPCM_H
#define _WX_SNDPCM_H

#include "wx/defs.h"
#include "wx/mmedia/defs.h"
#include "wx/mmedia/sndbase.h"

//
// PCM specification class
//

class wxSoundFormatPcm : public wxSoundFormatBase {
public:
  wxSoundFormatPcm(wxUint32 srate = 22500, wxUint8 bps = 8,
                   wxUint16 channels = 2, bool sign = true,
                   int order = wxLITTLE_ENDIAN); 
  ~wxSoundFormatPcm();

  void SetSampleRate(wxUint32 srate);
  void SetBPS(wxUint8 bps);
  void SetChannels(wxUint16 nchannels);
  void SetOrder(int order);
  void Signed(bool sign);

  wxUint32 GetSampleRate() const { return m_srate; }
  wxUint8 GetBPS() const { return m_bps; }
  wxUint16 GetChannels() const { return m_nchan; }
  int GetOrder() const { return m_order; }
  bool Signed() const { return m_signed; }

  wxSoundFormatType GetType() const { return wxSOUND_PCM; }
  wxSoundFormatBase *Clone() const;

  wxUint32 GetTimeFromBytes(wxUint32 bytes) const;
  wxUint32 GetBytesFromTime(wxUint32 time) const;

  bool operator!=(const wxSoundFormatBase& frmt2) const;

protected:
  wxUint32 m_srate;
  wxUint8 m_bps;
  wxUint16 m_nchan;
  int m_order;
  bool m_signed;
};

#endif
