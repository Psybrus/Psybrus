// --------------------------------------------------------------------------
// Name: sndesd.cpp
// Purpose:
// Date: 08/11/1999
// Author: Guilhem Lavaux <lavaux@easynet.fr> (C) 1999
// CVSID: $Id: sndesd.cpp 35650 2005-09-23 12:56:45Z MR $
// wxWindows licence
// --------------------------------------------------------------------------

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/string.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// --------------------------------------------------------------------------
// MMedia headers
// --------------------------------------------------------------------------

#include "wx/mmedia/sndbase.h"
#include "wx/mmedia/sndesd.h"
#include "wx/mmedia/sndpcm.h"

// --------------------------------------------------------------------------
// System headers
// --------------------------------------------------------------------------

#ifdef HAVE_ESD_H
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <esd.h>
#ifdef __WXGTK__
#include <gdk/gdk.h>
#endif
#endif

// --------------------------------------------------------------------------

#define MY_ESD_NAME "wxWidgets/wxSoundStreamESD"

// --------------------------------------------------------------------------
// wxSoundStreamESD: ESD sound driver

// --------------------------------------------------------------------------
// Constructors/Destructors
// --------------------------------------------------------------------------

wxSoundStreamESD::wxSoundStreamESD(const wxString& hostname)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return;
#else
    wxSoundFormatPcm pcm_default;

    // First, we make some basic test: is there ESD on this computer ?
    m_esd_ok = false;

    if (hostname.IsNull())
        m_fd_output = esd_play_stream(ESD_PLAY | ESD_STREAM, 22050,
                                      hostname.mb_str(), MY_ESD_NAME);
    else
        m_fd_output = esd_play_stream(ESD_PLAY | ESD_STREAM, 22050,
                                      NULL, MY_ESD_NAME);
    if (m_fd_output == -1) {
        // Answer: no. We return with an error.
        m_snderror = wxSOUND_INVDEV;
        return;
    }

    // Close this unuseful stream.
    esd_close(m_fd_output);

    m_hostname = hostname;

    // Set the default audio format
    SetSoundFormat(pcm_default);

    // Initialize some variable
    m_snderror = wxSOUND_NOERROR;
    m_esd_stop = true;
    m_q_filled = true;
    m_esd_ok   = true;
    m_fd_output= -1;
    m_fd_input = -1;
#endif // defined HAVE_ESD_H
}

wxSoundStreamESD::~wxSoundStreamESD()
{
#ifdef HAVE_ESD_H
    if (!m_esd_stop)
        StopProduction();
#endif // defined HAVE_ESD_H
}

// --------------------------------------------------------------------------
// Read several samples
// --------------------------------------------------------------------------

wxSoundStream& wxSoundStreamESD::Read(void *buffer, wxUint32 len)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return *this;
#else
    int ret;

    if (m_esd_stop) {
        m_snderror = wxSOUND_NOTSTARTED;
        return *this;
    }

    ret = read(m_fd_input, buffer, len);
    m_lastcount = (wxUint32)ret;

    if (ret < 0)
        m_snderror = wxSOUND_IOERROR;
    else
        m_snderror = wxSOUND_NOERROR;

    return *this;
#endif // defined HAVE_ESD_H
}

// --------------------------------------------------------------------------
// Write several samples
// --------------------------------------------------------------------------
wxSoundStream& wxSoundStreamESD::Write(const void *buffer, wxUint32 len)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return *this;
#else
    int ret;

    if (m_esd_stop) {
        m_lastcount = 0;
        m_snderror = wxSOUND_NOTSTARTED;
        return *this;
    }

    ret = write(m_fd_output, buffer, len);
    m_lastcount = (wxUint32)ret;

    if (ret < 0)
        m_snderror = wxSOUND_IOERROR;
  else
      m_snderror = wxSOUND_NOERROR;

    m_q_filled = true;

    return *this;
#endif // defined HAVE_ESD_H
}

// --------------------------------------------------------------------------
// SetSoundFormat(): this function specifies which format we want and which
// format is available
// --------------------------------------------------------------------------
bool wxSoundStreamESD::SetSoundFormat(const wxSoundFormatBase& format)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return false;
#else
    wxSoundFormatPcm *pcm_format;

    if (format.GetType() != wxSOUND_PCM) {
        m_snderror = wxSOUND_INVFRMT;
        return false;
    }

    if (!m_esd_ok) {
        m_snderror = wxSOUND_INVDEV;
        return false;
    }

    if (m_sndformat)
        delete m_sndformat;

    m_sndformat = format.Clone();
    if (!m_sndformat) {
        m_snderror = wxSOUND_MEMERROR;
        return false;
    }
    pcm_format = (wxSoundFormatPcm *)m_sndformat;

    // Detect the best format
    DetectBest(pcm_format);

    m_snderror = wxSOUND_NOERROR;
    if (*pcm_format != format) {
        m_snderror = wxSOUND_NOEXACT;
        return false;
    }
    return true;
#endif // defined HAVE_ESD_H
}

// --------------------------------------------------------------------------
// _wxSound_OSS_CBack (internal): it is called when the driver (ESD) is
// ready for a next buffer.
// --------------------------------------------------------------------------
#if defined(__WXGTK__) && defined(HAVE_ESD_H)
static void _wxSound_OSS_CBack(gpointer data, int source,
                               GdkInputCondition condition)
{
    wxSoundStreamESD *esd = (wxSoundStreamESD *)data;

    switch (condition) {
        case GDK_INPUT_READ:
            esd->WakeUpEvt(wxSOUND_INPUT);
            break;
        case GDK_INPUT_WRITE:
            esd->WakeUpEvt(wxSOUND_OUTPUT);
            break;
        default:
            break;
    }
}
#endif


// --------------------------------------------------------------------------
// WakeUpEvt() (internal): it is called by _wxSound_OSS_CBack to bypass the
// C++ protection
// --------------------------------------------------------------------------
void wxSoundStreamESD::WakeUpEvt(int evt)
{
    m_q_filled = false;
    OnSoundEvent(evt);
}

// --------------------------------------------------------------------------
// StartProduction(): see wxSoundStream
// --------------------------------------------------------------------------
bool wxSoundStreamESD::StartProduction(int evt)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return false;
#else
    wxSoundFormatPcm *pcm;
    int flag = 0;

    if (!m_esd_ok) {
        m_snderror = wxSOUND_INVDEV;
        return false;
    }

    if (!m_esd_stop)
        StopProduction();

    pcm = (wxSoundFormatPcm *)m_sndformat;

    flag |= (pcm->GetBPS() == 16) ? ESD_BITS16 : ESD_BITS8;
    flag |= (pcm->GetChannels() == 2) ? ESD_STEREO : ESD_MONO;

    if ((evt & wxSOUND_OUTPUT) != 0) {
        flag |= ESD_PLAY | ESD_STREAM;
        m_fd_output = esd_play_stream(flag, pcm->GetSampleRate(), NULL,
                                      MY_ESD_NAME);
    }

    if ((evt & wxSOUND_INPUT) != 0) {
        flag |= ESD_RECORD | ESD_STREAM;
        m_fd_input = esd_record_stream(flag, pcm->GetSampleRate(), NULL,
                                       MY_ESD_NAME);
    }

#ifdef __WXGTK__
    if ((evt & wxSOUND_OUTPUT) != 0) {
        m_tag_output = gdk_input_add(m_fd_output, GDK_INPUT_WRITE,
                                     _wxSound_OSS_CBack, (gpointer)this);
    }
    if ((evt & wxSOUND_INPUT) != 0) {
        m_tag_input = gdk_input_add(m_fd_input, GDK_INPUT_READ,
                                    _wxSound_OSS_CBack, (gpointer)this);
    }
#endif

    m_esd_stop = false;
    m_q_filled = false;

    return true;
#endif // defined HAVE_ESD_H
}

// --------------------------------------------------------------------------
// StopProduction(): see wxSoundStream
// --------------------------------------------------------------------------
bool wxSoundStreamESD::StopProduction()
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return false;
#else
    if (m_esd_stop)
        return false;

    if (m_fd_input != -1) {
        esd_close(m_fd_input);
#ifdef __WXGTK__
        gdk_input_remove(m_tag_input);
#endif
    }
    if (m_fd_output != -1) {
        esd_close(m_fd_output);
#ifdef __WXGTK__
        gdk_input_remove(m_tag_output);
#endif
    }

    m_fd_input = -1;
    m_fd_output= -1;
    m_esd_stop = true;
    m_q_filled = true;
    return true;
#endif // defined HAVE_ESD_H
}

//
// Detect the closest format (The best).
//
void wxSoundStreamESD::DetectBest(wxSoundFormatPcm *pcm)
{
#ifndef HAVE_ESD_H
    m_snderror = wxSOUND_INVDEV;
    return;
#else
    wxSoundFormatPcm best_pcm;

    // We change neither the number of channels nor the sample rate
    // because ESD is clever.

    best_pcm.SetSampleRate(pcm->GetSampleRate());
    best_pcm.SetChannels(pcm->GetChannels());

    // It supports 16 bits
    if (pcm->GetBPS() >= 16)
        best_pcm.SetBPS(16);
    else
        best_pcm.SetBPS(8);

    best_pcm.SetOrder(wxLITTLE_ENDIAN);
    best_pcm.Signed(true);

    // Finally recopy the new format
    *pcm = best_pcm;
#endif // defined HAVE_ESD_H
}

