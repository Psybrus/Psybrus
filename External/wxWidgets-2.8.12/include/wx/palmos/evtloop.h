///////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/evtloop.h
// Purpose:     wxEventLoop class for Palm OS
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     2004-10-14
// RCS-ID:      $Id: evtloop.h 31469 2005-01-18 21:14:27Z ABX $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALMOS_EVTLOOP_H_
#define _WX_PALMOS_EVTLOOP_H_

// ----------------------------------------------------------------------------
// wxEventLoop
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxEventLoop : public wxEventLoopBase
{
public:
    wxEventLoop();

    // implement base class pure virtuals
    virtual int Run();
    virtual void Exit(int rc = 0);
    virtual bool Pending() const;
    virtual bool Dispatch();
    virtual bool IsRunning() const;

    // MSW-specific methods
    // --------------------

    // preprocess a message, return true if processed (i.e. no further
    // dispatching required)
    virtual bool PreProcessMessage(WXMSG *msg);

    // process a single message
    virtual void ProcessMessage(WXMSG *msg);

protected:
    // should we exit the loop?
    bool m_shouldExit;

    // the loop exit code
    int m_exitcode;
};

#endif // _WX_PALMOS_EVTLOOP_H_

