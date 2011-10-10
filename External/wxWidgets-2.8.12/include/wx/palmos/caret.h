///////////////////////////////////////////////////////////////////////////////
// Name:        wx/palmos/caret.h
// Purpose:     wxCaret class - the Palm OS implementation of wxCaret
// Author:      William Osborne - minimal working wxPalmOS port
// Modified by:
// Created:     10/13/04
// RCS-ID:      $Id: caret.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) William Osborne
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CARET_H_
#define _WX_CARET_H_

class WXDLLEXPORT wxCaret : public wxCaretBase
{
public:
    wxCaret() { Init(); }
        // create the caret of given (in pixels) width and height and associate
        // with the given window
    wxCaret(wxWindow *window, int width, int height)
    {
        Init();

        (void)Create(window, width, height);
    }
        // same as above
    wxCaret(wxWindowBase *window, const wxSize& size)
    {
        Init();

        (void)Create(window, size);
    }

    // process wxWindow notifications
    virtual void OnSetFocus();
    virtual void OnKillFocus();

protected:
    void Init()
    {
        wxCaretBase::Init();

        m_hasCaret = FALSE;
    }

    // override base class virtuals
    virtual void DoMove();
    virtual void DoShow();
    virtual void DoHide();
    virtual void DoSize();

    // helper function which creates the system caret
    bool PalmOSCreateCaret();

private:
    bool m_hasCaret;

    DECLARE_NO_COPY_CLASS(wxCaret)
};

#endif // _WX_CARET_H_


