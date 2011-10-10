/////////////////////////////////////////////////////////////////////////////
// Name:        docview.h
// Purpose:     Document/view demo
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: docview.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __DOCVIEWSAMPLEH__
#define __DOCVIEWSAMPLEH__

#include "wx/docview.h"

class wxDocManager;

// Define a new application
class MyApp: public wxApp
{
public:
    MyApp(void);
    bool OnInit(void);
    int OnExit(void);
    
    wxFrame *CreateChildFrame(wxDocument *doc, wxView *view, bool isCanvas);
    
protected:
    wxDocManager* m_docManager;
};

DECLARE_APP(MyApp)

// Define a new frame
class MyCanvas;
class MyFrame: public wxDocParentFrame
{
    DECLARE_CLASS(MyFrame)
public:
    wxMenu *editMenu;
    
    // This pointer only needed if in single window mode
    MyCanvas *canvas;
    
    MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
        const long type);
    
    void OnAbout(wxCommandEvent& event);
    MyCanvas *CreateCanvas(wxView *view, wxFrame *parent);
    
    DECLARE_EVENT_TABLE()
};

extern MyFrame *GetMainFrame(void);

#define DOCVIEW_CUT     1
#define DOCVIEW_ABOUT   wxID_ABOUT

extern bool singleWindowMode;

#endif
