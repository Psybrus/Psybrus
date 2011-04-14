/////////////////////////////////////////////////////////////////////////////
// Name:        penguin.h
// Purpose:     wxGLCanvas demo program
// Author:      Robert Roebling
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: penguin.h 36174 2005-11-14 17:38:24Z JS $
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PENGUIN_H_
#define _WX_PENGUIN_H_


#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/wfstream.h"
#if wxUSE_ZLIB
#include "wx/zstream.h"
#endif

#include "wx/glcanvas.h"

extern "C"
{
#include "trackball.h"
}

#include "dxfrenderer.h"

// OpenGL view data
struct GLData
{
    bool initialized;           // have OpenGL been initialized?
    float beginx, beginy;       // position of mouse
    float quat[4];              // orientation of object
    float zoom;                 // field of view in degrees
};

// Define a new application type
class MyApp: public wxApp
{
public:
    bool OnInit();
};

// Define a new frame type
class TestGLCanvas;

class MyFrame: public wxFrame
{
public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

    void OnMenuFileOpen(wxCommandEvent& event);
    void OnMenuFileExit(wxCommandEvent& event);
    void OnMenuHelpAbout(wxCommandEvent& event);

#if wxUSE_GLCANVAS
    void SetCanvas( TestGLCanvas *canvas ) { m_canvas = canvas; }
    TestGLCanvas *GetCanvas() { return m_canvas; }

private:
    TestGLCanvas *m_canvas;
#endif

    DECLARE_EVENT_TABLE()
};

#if wxUSE_GLCANVAS

class TestGLCanvas: public wxGLCanvas
{
public:
    TestGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxString& name = wxT("TestGLCanvas"));

    ~TestGLCanvas();

    void LoadDXF(const wxString& filename);

protected:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouse(wxMouseEvent& event);

private:
    void InitGL();
    void ResetProjectionMode();

    GLData m_gldata;
    DXFRenderer m_renderer;

    DECLARE_EVENT_TABLE()
};

#endif // #if wxUSE_GLCANVAS

#endif // #ifndef _WX_PENGUIN_H_

