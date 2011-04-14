///////////////////////////////////////////////////////////////////////////////
// Name:        wx/os2/apptbase.h
// Purpose:     declaration of wxAppTraits for OS2
// Author:      Stefan Neis
// Modified by:
// Created:     22.09.2003
// RCS-ID:      $Id: apptbase.h 27408 2004-05-23 20:53:33Z JS $
// Copyright:   (c) 2003 Stefan Neis <Stefan.Neis@t-online.de>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OS2_APPTBASE_H_
#define _WX_OS2_APPTBASE_H_

// ----------------------------------------------------------------------------
// wxAppTraits: the OS2 version adds extra hooks needed by OS2-only code
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxAppTraits : public wxAppTraitsBase
{
public:
    // wxThread helpers
    // ----------------

    // Initialize PM facilities to enable GUI access
    virtual void InitializeGui(unsigned long &ulHab);

    // Clean up message queue.
    virtual void TerminateGui(unsigned long ulHab);
};

#endif // _WX_OS2_APPTBASE_H_
