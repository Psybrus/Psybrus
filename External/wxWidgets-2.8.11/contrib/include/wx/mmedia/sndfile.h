// --------------------------------------------------------------------------
// Name: sndfile.h
// Purpose:
// Date: 08/11/1999
// Author: Guilhem Lavaux <lavaux@easynet.fr> (C) 1999
// CVSID: $Id: sndfile.h 34412 2005-05-30 12:39:25Z MW $
// License:    wxWindows license
// --------------------------------------------------------------------------
#ifndef _WX_SNDFILE_H
#define _WX_SNDFILE_H

#include "wx/defs.h"
#include "wx/stream.h"
#include "wx/mmedia/defs.h"
#include "wx/mmedia/sndbase.h"
#include "wx/mmedia/sndcodec.h"

#define wxSOUND_INFINITE_TIME ((wxUint32)-1)

//
// Codec router class
//

class WXDLLIMPEXP_MMEDIA wxSoundRouterStream: public wxSoundStreamCodec {
 public:
  wxSoundRouterStream(wxSoundStream& sndio);
  ~wxSoundRouterStream();

  wxSoundStream& Read(void *buffer, wxUint32 len);
  wxSoundStream& Write(const void *buffer, wxUint32 len);

  bool SetSoundFormat(const wxSoundFormatBase& format);

  bool StartProduction(int evt);
  bool StopProduction();

  wxUint32 GetBestSize() const;

 protected:
  wxSoundStream *m_router;
};

typedef enum {
 wxSOUND_FILE_STOPPED,
 wxSOUND_FILE_PAUSED,
 wxSOUND_FILE_PLAYING,
 wxSOUND_FILE_RECORDING
} wxSoundFileState;

//
// Base class for file coders/decoders
//

class WXDLLIMPEXP_MMEDIA wxSoundFileStream: public wxSoundStream {
public:
    wxSoundFileStream(wxInputStream& stream, wxSoundStream& io_sound);
    wxSoundFileStream(wxOutputStream& stream, wxSoundStream& io_sound);
    ~wxSoundFileStream();
    
    // Usual sound file calls (Play, Stop, ...)
    bool Play();
    bool Record(wxUint32 time);
    bool Stop();
    bool Pause();
    bool Resume();
    
    // Functions which return the current state
    bool IsStopped() const { return m_state == wxSOUND_FILE_STOPPED; }
    bool IsPaused() const { return m_state == wxSOUND_FILE_PAUSED; }
    
    // A user should not call these two functions.
    // Several things must be done before calling them.
    // Users should use Play(), ... 
    bool StartProduction(int evt);
    bool StopProduction();

    // These three functions deals with the length, the position in the sound file.
    // All the values are expressed in bytes. If you need the values expressed
    // in terms of time, you have to use GetSoundFormat().GetTimeFromBytes(...)
    wxUint32 GetLength();
    wxUint32 GetPosition();
    wxUint32 SetPosition(wxUint32 new_position);
    
    // These two functions use the sound format specified by GetSoundFormat().
    // All samples must be encoded in that format. 
    wxSoundStream& Read(void *buffer, wxUint32 len); 
    wxSoundStream& Write(const void *buffer, wxUint32 len);
    
    // This function set the sound format of the file. !! It must be used only
    // when you are in output mode (concerning the file) !! If you are in
    // input mode (concerning the file) you can't use this function to modify
    // the format of the samples returned by Read() !
    // For this action, you must use wxSoundRouterStream applied to wxSoundFileStream. 
    bool SetSoundFormat(const wxSoundFormatBase& format);
    
    // This function returns the Codec name. This is useful for those who want to build
    // a player (But also in some other case).
    virtual wxString GetCodecName() const;
    
    // You should use this function to test whether this file codec can read
    // the stream you passed to it.
    virtual bool CanRead() { return false; }
    
protected:
    wxSoundRouterStream m_codec; 
    wxSoundStream *m_sndio;
    wxInputStream *m_input;
    wxOutputStream *m_output;
    
    wxSoundFileState m_state, m_oldstate;
    wxUint32 m_length, m_bytes_left;
    bool m_prepared;
    
protected:
    virtual bool PrepareToPlay() = 0; 
    virtual bool PrepareToRecord(wxUint32 time) = 0;
    virtual bool FinishRecording() = 0;
    virtual bool RepositionStream(wxUint32 position) = 0;
    void FinishPreparation(wxUint32 len);
    
    virtual wxUint32 GetData(void *buffer, wxUint32 len) = 0;
    virtual wxUint32 PutData(const void *buffer, wxUint32 len) = 0;
    
    void OnSoundEvent(int evt);
};

#endif
