/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/app.h
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: app.h 43879 2006-12-09 17:46:20Z PC $
// Copyright:   (c) 1998 Robert Roebling, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_APP_H_
#define _WX_GTK_APP_H_

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxApp;
class WXDLLIMPEXP_BASE wxLog;

//-----------------------------------------------------------------------------
// wxApp
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxApp: public wxAppBase
{
public:
    wxApp();
    virtual ~wxApp();

    /* override for altering the way wxGTK intializes the GUI
     * (palette/visual/colorcube). under wxMSW, OnInitGui() does nothing by
     * default. when overriding this method, the code in it is likely to be
     * platform dependent, otherwise use OnInit(). */
    virtual bool OnInitGui();

    // override base class (pure) virtuals
    virtual bool Yield(bool onlyIfNeeded = FALSE);
    virtual void WakeUpIdle();

    virtual bool Initialize(int& argc, wxChar **argv);
    virtual void CleanUp();

    static bool InitialzeVisual();

#ifdef __WXDEBUG__
    virtual void OnAssertFailure(const wxChar *file,
                                 int line,
                                 const wxChar *func,
                                 const wxChar *cond,
                                 const wxChar *msg);

    bool IsInAssert() const { return m_isInAssert; }
#endif // __WXDEBUG__

    guint m_idleTag;
    // temporarily disable idle events
    void SuspendIdleCallback();

    // Used by the the wxGLApp and wxGLCanvas class for GL-based X visual
    // selection.
    void           *m_glVisualInfo; // this is actually an XVisualInfo*
    void           *m_glFBCInfo; // this is actually an GLXFBConfig*
    // This returns the current visual: either that used by wxRootWindow
    // or the XVisualInfo* for SGI.
    GdkVisual      *GetGdkVisual();
    
private:
    // true if we're inside an assert modal dialog
#ifdef __WXDEBUG__
    bool m_isInAssert;
#endif // __WXDEBUG__

    DECLARE_DYNAMIC_CLASS(wxApp)
    DECLARE_EVENT_TABLE()
};

#endif // _WX_GTK_APP_H_
