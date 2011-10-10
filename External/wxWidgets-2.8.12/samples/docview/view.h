/////////////////////////////////////////////////////////////////////////////
// Name:        view.h
// Purpose:     View classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: view.h 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __VIEWSAMPLEH__
#define __VIEWSAMPLEH__

#include "wx/docview.h"

class MyCanvas: public wxScrolledWindow
{
public:
    wxView *view;
    
    MyCanvas(wxView *v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
    virtual void OnDraw(wxDC& dc);
    void OnMouseEvent(wxMouseEvent& event);
    
    DECLARE_EVENT_TABLE()
};

class MyTextWindow: public wxTextCtrl
{
public:
    wxView *view;
    
    MyTextWindow(wxView *v, wxFrame *frame, const wxPoint& pos, const wxSize& size, const long style);
};

class DrawingView: public wxView
{
    DECLARE_DYNAMIC_CLASS(DrawingView)
private:
public:
    wxFrame *frame;
    MyCanvas *canvas;
    
    DrawingView(void) { canvas = (MyCanvas *) NULL; frame = (wxFrame *) NULL; };
    ~DrawingView(void) {};
    
    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = true);
    
    void OnCut(wxCommandEvent& event);
    
    DECLARE_EVENT_TABLE()
};

class TextEditView: public wxView
{
    DECLARE_DYNAMIC_CLASS(TextEditView)
private:
public:
    wxFrame *frame;
    MyTextWindow *textsw;
    
    TextEditView(): wxView() { frame = (wxFrame *) NULL; textsw = (MyTextWindow *) NULL; }
    ~TextEditView(void) {}
    
    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = true);
};

#endif
