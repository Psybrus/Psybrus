/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/toplevel.h
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: toplevel.h 43846 2006-12-07 05:50:54Z PC $
// Copyright:   (c) 1998 Robert Roebling, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_TOPLEVEL_H_
#define _WX_GTK_TOPLEVEL_H_

//-----------------------------------------------------------------------------
// wxTopLevelWindowGTK
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxTopLevelWindowGTK : public wxTopLevelWindowBase
{
public:
    // construction
    wxTopLevelWindowGTK() { Init(); }
    wxTopLevelWindowGTK(wxWindow *parent,
                        wxWindowID id,
                        const wxString& title,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxDEFAULT_FRAME_STYLE,
                        const wxString& name = wxFrameNameStr)
    {
        Init();

        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxFrameNameStr);

    virtual ~wxTopLevelWindowGTK();

    // implement base class pure virtuals
    virtual void Maximize(bool maximize = true);
    virtual bool IsMaximized() const;
    virtual void Iconize(bool iconize = true);
    virtual bool IsIconized() const;
    virtual void SetIcon(const wxIcon& icon);
    virtual void SetIcons(const wxIconBundle& icons);
    virtual void Restore();

    virtual bool EnableCloseButton(bool enable = true);

    virtual bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL);
    virtual bool IsFullScreen() const { return m_fsIsShowing; };

    virtual bool SetShape(const wxRegion& region);

    virtual void RequestUserAttention(int flags = wxUSER_ATTENTION_INFO);

    virtual void SetWindowStyleFlag( long style );

    virtual bool Show(bool show = true);

    virtual void Raise();

    virtual bool IsActive();

    virtual void SetTitle( const wxString &title );
    virtual wxString GetTitle() const { return m_title; }

    virtual bool SetTransparent(wxByte alpha);
    virtual bool CanSetTransparent();

    // Experimental, to allow help windows to be
    // viewable from within modal dialogs
    virtual void AddGrab();
    virtual void RemoveGrab();
    virtual bool IsGrabbed() const { return m_grabbed; }

    // implementation from now on
    // --------------------------

    // GTK callbacks
    virtual void GtkOnSize();
    virtual void OnInternalIdle();

    // do *not* call this to iconize the frame, this is a private function!
    void SetIconizeState(bool iconic);

    int           m_miniEdge,
                  m_miniTitle;
    GtkWidget    *m_mainWidget;
    bool          m_insertInClientArea;  /* not from within OnCreateXXX */

    bool          m_fsIsShowing;         /* full screen */
    long          m_fsSaveGdkFunc, m_fsSaveGdkDecor;
    long          m_fsSaveFlag;
    wxRect        m_fsSaveFrame;

    // m_windowStyle translated to GDK's terms
    long          m_gdkFunc,
                  m_gdkDecor;

    // private gtk_timeout_add result for mimicing wxUSER_ATTENTION_INFO and
    // wxUSER_ATTENTION_ERROR difference, -2 for no hint, -1 for ERROR hint, rest for GtkTimeout handle.
    int m_urgency_hint;

    // give hints to the Window Manager for how the size
    // of the TLW can be changed by dragging
    virtual void DoSetSizeHints( int minW, int minH,
                                 int maxW = wxDefaultCoord, int maxH = wxDefaultCoord,
                                 int incW = wxDefaultCoord, int incH = wxDefaultCoord );

protected:
    // common part of all ctors
    void Init();

    // move the window to the specified location and resize it: this is called
    // from both DoSetSize() and DoSetClientSize()
    virtual void DoMoveWindow(int x, int y, int width, int height);

    // override wxWindow methods to take into account tool/menu/statusbars
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO);
                                 
    virtual void DoSetClientSize(int width, int height);
    virtual void DoGetClientSize( int *width, int *height ) const;

    wxString      m_title;

    // is the frame currently iconized?
    bool m_isIconized;
    // is the frame currently grabbed explicitly
    // by the application?
    bool m_grabbed;
};

#endif // _WX_GTK_TOPLEVEL_H_
