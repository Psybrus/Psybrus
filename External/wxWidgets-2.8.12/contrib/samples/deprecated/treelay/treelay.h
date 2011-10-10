///////////////////////////////////////////////////////////////////////////////
// Name:        treelay.h
// Purpose:     wxTreeLayout sample
// Author:      Julian Smart
// Modified by:
// Created:     7/4/98
// RCS-ID:      $Id: treelay.h 29463 2004-09-27 19:24:45Z ABX $
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// Define a new application
class MyApp: public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
    void TreeTest(wxTreeLayoutStored& tree, wxDC& dc);
};

DECLARE_APP(MyApp)

class MyCanvas;

class MyFrame: public wxFrame
{
  public:
    MyCanvas *canvas;
    MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnCloseWindow(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnLeftRight(wxCommandEvent& event);
    void OnTopBottom(wxCommandEvent& event);

DECLARE_EVENT_TABLE()
};

// Define a new canvas which can receive some events
class MyCanvas: public wxScrolledWindow
{
  public:
    MyCanvas(wxWindow *frame);
    void OnPaint(wxPaintEvent& event);
    void OnEvent(wxMouseEvent& event);
    void OnChar(wxKeyEvent& event);
DECLARE_EVENT_TABLE()
};

#define TEST_QUIT      1
#define TEST_ABOUT     2
#define TEST_LEFT_RIGHT 3
#define TEST_TOP_BOTTOM 4

