///////////////////////////////////////////////////////////////////////////////
// Name:        wx/os2/apptrait.h
// Purpose:     class implementing wxAppTraits for OS/2
// Author:      Stefan Neis
// Modified by:
// Created:     22.09.2003
// RCS-ID:      $Id: apptrait.h 53707 2008-05-22 21:19:43Z SN $
// Copyright:   (c) 2003 Stefan Neis <Stefan.Neis@t-online.de>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OS2_APPTRAIT_H_
#define _WX_OS2_APPTRAIT_H_

// ----------------------------------------------------------------------------
// wxGUI/ConsoleAppTraits: must derive from wxAppTraits, not wxAppTraitsBase
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxConsoleAppTraits : public wxConsoleAppTraitsBase
{
public:
};

#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxGUIAppTraits : public wxGUIAppTraitsBase
{
public:
    virtual wxPortId GetToolkitVersion(int *majVer, int *minVer) const;

    // wxThread helpers
    // ----------------

    // Initialize PM facilities to enable GUI access
    virtual void InitializeGui(unsigned long &ulHab);

    // Clean up message queue.
    virtual void TerminateGui(unsigned long ulHab);
#ifdef __WXGTK__
    virtual wxString GetDesktopEnvironment() const;
#endif
};

#ifndef __WXPM__
inline void wxGUIAppTraits::InitializeGui(unsigned long &WXUNUSED(ulHab))
{
}

inline void wxGUIAppTraits::TerminateGui(unsigned long WXUNUSED(ulHab))
{
}
#endif

#endif // wxUSE_GUI

#endif // _WX_OS2_APPTRAIT_H_
