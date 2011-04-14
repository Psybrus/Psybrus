/////////////////////////////////////////////////////////////////////////////
// Name:        timer.h
// Purpose:     wxTimer class
// Author:      David Webster
// Modified by:
// Created:     10/17/99
// RCS-ID:      $Id: timer.h 41020 2006-09-05 20:47:48Z VZ $
// Copyright:   (c) David Webster
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TIMER_H_
#define _WX_TIMER_H_

#include "wx/object.h"

class WXDLLEXPORT wxTimer: public wxTimerBase
{
friend void wxProcessTimer(wxTimer& timer);

public:
    wxTimer() { Init(); }
    wxTimer( wxEvtHandler* pOwner
            ,int           nId = -1
           )
           : wxTimerBase( pOwner
                         ,nId
                        )
        { Init(); }
    virtual ~wxTimer();

    virtual void Notify(void);
    virtual bool Start( int  nMilliseconds = -1
                       ,bool bOneShot = FALSE
                      );
    virtual void Stop(void);

    inline virtual bool IsRunning(void) const { return m_ulId != 0L; }
    inline          int GetTimerId(void) const { return m_idTimer; }

protected:
    void Init(void);

    ULONG                            m_ulId;
    HAB                              m_Hab;

private:
    DECLARE_ABSTRACT_CLASS(wxTimer)
};

extern ULONG wxTimerProc( HWND  WXUNUSED(hwnd)
                         ,ULONG
                         ,int   nIdTimer
                         ,ULONG
                        );
#endif
    // _WX_TIMER_H_
